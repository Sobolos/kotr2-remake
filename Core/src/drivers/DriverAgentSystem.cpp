#include "kotr/drivers/DriverAgentSystem.hpp"
#include "kotr/vehicles/VehicleSystem.hpp"
#include <algorithm>
#include <cmath>

namespace kotr::drivers {

    using namespace kotr::core;
    using namespace kotr::contracts;
    using namespace kotr::economy;

    DriverAgentSystem::DriverAgentSystem(EventBus& eventBus,
        EconomySystem& econ,
        ContractSystem& contractSys,
        DriverSystem& driverSys,
        TimeSystem& time)
        : eventBus_(eventBus), economy_(econ), contracts_(contractSys),
        drivers_(driverSys), time_(time) {
        eventBus_.subscribe<GameHourElapsed>([this](const GameHourElapsed& e) {
            updateHour(e.time);
            });

        // Доставка завершена: агент свободен, база = назначение, score и деньги
        eventBus_.subscribe<ContractDeliveryArrivedEvent>(
            [this](const ContractDeliveryArrivedEvent& e) {
                auto it = agents_.find(e.carrierId);
                if (it != agents_.end()) {
                    if (vehicles_ && !it->second.vehicleRef.empty()) {
                        vehicles_->setCargoLoad(it->second.vehicleRef, 0.0);
                    }

                    it->second.busy = false;
                    const auto* c = contracts_.getContract(e.contractId);
                    if (c) it->second.currentBase = c->destination;
                    it->second.deliveryScore += e.massTons * e.distanceKm / 100.0;
                }
                // Выручка идёт работодателю (player обрабатывается внешней разводкой)
                if (!e.employerId.empty() && e.employerId != "player") {
                    auto emp = agents_.find(e.employerId);
                    if (emp != agents_.end()) emp->second.money += e.payout;
                }
            });

        // Найм: агент переходит к работодателю; именной-наниматель платит стоимость
        eventBus_.subscribe<DriverHiredEvent>([this](const DriverHiredEvent& e) {
            auto it = agents_.find(e.driverId);
            if (it != agents_.end()) {
                it->second.employerId = e.employerId;
                it->second.busy = false;
            }
            if (e.employerId != "player") {
                auto emp = agents_.find(e.employerId);
                if (emp != agents_.end()) emp->second.money -= e.totalCost;
            }
            });

        eventBus_.subscribe<DriverFiredEvent>([this](const DriverFiredEvent& e) {
            auto it = agents_.find(e.driverId);
            if (it != agents_.end()) it->second.employerId = "self";
            });

        eventBus_.subscribe<DriverVehicleDestroyedEvent>([this](const DriverVehicleDestroyedEvent& e) {
            auto it = agents_.find(e.driverId);
            if (it != agents_.end()) { it->second.active = false; it->second.busy = false; }
            });

        eventBus_.subscribe<DriverReturnedEvent>([this](const DriverReturnedEvent& e) {
            auto it = agents_.find(e.driverId);
            if (it != agents_.end()) {
                it->second.active = true;
                it->second.capacityKg = e.ersatzCapacityKg;
            }
            });

        // Зарплаты: именной-наниматель платит своим нанятым (player — внешняя разводка)
        eventBus_.subscribe<DriverSalaryDueEvent>([this](const DriverSalaryDueEvent& e) {
            if (e.employerId.empty() || e.employerId == "player") return;
            auto emp = agents_.find(e.employerId);
            if (emp != agents_.end()) emp->second.money -= e.amount;
            });
    }

    // === Регистрация ===

    void DriverAgentSystem::addMassAI(const std::string& id, double capacityKg,
        double aggression, CityId home) {
        DriverAgent a;
        a.id = id;
        a.kind = AgentKind::MassAI;
        a.capacityKg = capacityKg;
        a.aggression = aggression;
        a.currentBase = std::move(home);
        agents_[id] = a;
    }

    void DriverAgentSystem::addNamedAgent(const std::string& driverId, double capacityKg,
        double aggression, CityId home) {
        DriverAgent a;
        a.id = driverId;
        a.kind = AgentKind::Named;
        a.capacityKg = capacityKg;
        a.aggression = aggression;
        a.currentBase = std::move(home);
        agents_[driverId] = a;
    }

    // === Симуляция ===

    void DriverAgentSystem::updateHour(GameTime now) {
        // Детерминированный обход
        std::vector<std::string> ids;
        ids.reserve(agents_.size());
        for (const auto& [id, a] : agents_) ids.push_back(id);
        std::sort(ids.begin(), ids.end());

        for (const auto& id : ids) {
            auto& a = agents_[id];
            if (!a.active || a.busy) continue;
            tryTakeFor(a, now);
        }
    }

    void DriverAgentSystem::tryTakeFor(DriverAgent& a, GameTime now) {
        const bool guaranteed = (a.failedTakeAttempts >= MERCY_VISITS);

        // Ёмкость и скорость — от тягача, если он назначен
        double capKg = a.capacityKg;
        double emptySpeed = a.speedKmh;
        if (vehicles_ && !a.vehicleRef.empty()) {
            double vc = vehicles_->getMarketCapacityKg(a.vehicleRef);
            if (vc > 0.0) capKg = vc;
            emptySpeed = vehicles_->calcEffectiveSpeedKmh(a.vehicleRef, 0.0);
            if (emptySpeed <= 0.0) emptySpeed = a.speedKmh;
        }

        // Локальная копия: указатели на offer живут до конца функции
        auto offers = contracts_.getAvailableContracts();

        const contracts::ContractOffer* best = nullptr;
        double bestScore = -1.0;
        double bestInterest = 0.0;
        GameMinutes bestDelay = 0;
        CityId bestOrigin;

        for (const auto& offer : offers) {
            GameMinutes delay = 0;
            double pref = 1.0;

            if (offer.origin == a.currentBase) {
                delay = 0;
            }
            else if (isNeighbor(a.currentBase, offer.origin)) {
                delay = static_cast<GameMinutes>(travelMinutes(a.currentBase, offer.origin, emptySpeed));
                pref = 0.8;
            }
            else {
                continue;
            }

            double remaining = contracts_.remainingTons(offer);
            double mass = std::min(capKg / 1000.0, remaining);
            if (mass < 2.0) continue;

            double deficit = economy_.getDeficitRatio(offer.destination, offer.good);
            double interest = std::clamp((deficit - 0.8) * 0.5, 0.0, 1.0);

            double score = interest * pref;
            if (score > bestScore) {
                bestScore = score;
                bestInterest = interest;
                best = &offer;
                bestDelay = delay;
                bestOrigin = offer.origin;
            }
        }

        if (best) {
            std::uniform_real_distribution<double> dist(0.0, 1.0);
            double chance = bestInterest * a.aggression * baseTakeRate_;
            bool take = guaranteed || (bestInterest >= MIN_TAKE_INTEREST && dist(rng_) < chance);

            if (take) {
                double remaining = contracts_.remainingTons(*best);
                double mass = std::min(capKg / 1000.0, remaining);

                // Гружёная машина едет медленнее (по Vehicle Design §13.2)
                double loadedSpeed = emptySpeed;
                if (vehicles_ && !a.vehicleRef.empty()) {
                    double vs = vehicles_->calcEffectiveSpeedKmh(a.vehicleRef, mass * 1000.0);
                    if (vs > 0.0) loadedSpeed = vs;
                }

                CarrierRef ref;
                ref.id = a.id;
                ref.kind = (a.kind == AgentKind::Named) ? CarrierKind::Named : CarrierKind::MassAI;
                ref.employerId = (a.employerId == "self") ? a.id : a.employerId;
                ref.speedKmh = loadedSpeed;
                ref.capacityKg = capKg;

                if (contracts_.joinContract(best->id, ref, mass, bestDelay)) {
                    a.busy = true;
                    a.failedTakeAttempts = 0;
                    a.currentBase = bestOrigin;
                    if (vehicles_ && !a.vehicleRef.empty()) {
                        vehicles_->setCargoLoad(a.vehicleRef, mass * 1000.0);
                    }
                    return;
                }
            }
        }

        a.failedTakeAttempts++;
        moveToBestNeighbor(a);
    }

    void DriverAgentSystem::moveToBestNeighbor(DriverAgent& a) {
        // Считаем, у какой соседней базы больше активных контрактов-истоков
        std::unordered_map<CityId, int> originCounts;
        for (const auto& offer : contracts_.getAvailableContracts()) {
            originCounts[offer.origin]++;
        }

        CityId bestCity;
        int bestCount = 0;
        for (const auto& [city, count] : originCounts) {
            if (city != a.currentBase && isNeighbor(a.currentBase, city) && count > bestCount) {
                bestCount = count;
                bestCity = city;
            }
        }
        if (!bestCity.empty()) {
            a.currentBase = bestCity; // переезд в пределах часа — допустимо для экон. симуляции
        }
    }

    // === Запросы ===

    const DriverAgent* DriverAgentSystem::getAgent(const std::string& id) const {
        auto it = agents_.find(id);
        return it != agents_.end() ? &it->second : nullptr;
    }

    double DriverAgentSystem::getCompetitorCapacityKg() const {
        double total = 0.0;
        for (const auto& [id, a] : agents_) {
            if (!a.active) continue;
            if (a.kind == AgentKind::MassAI) {
                total += a.capacityKg; // массовка всегда независима
            }
            else if (a.employerId == "self") {
                // самозанятый именной: своя ёмкость + ёмкость своих нанятых
                total += a.capacityKg;
                for (const auto& [hid, h] : agents_) {
                    if (h.kind == AgentKind::Named && h.employerId == id && h.active) {
                        total += h.capacityKg;
                    }
                }
            }
            // нанятые игроком уходят в ёмкость игрока (внешняя разводка MarketSystem)
        }
        return total;
    }

    double DriverAgentSystem::getDeliveryScore(const std::string& id) const {
        auto it = agents_.find(id);
        return it != agents_.end() ? it->second.deliveryScore : 0.0;
    }

    // === География ===

    double DriverAgentSystem::straightDistanceKm(const CityId& a, const CityId& b) const {
        const auto* ca = economy_.getCityData(a);
        const auto* cb = economy_.getCityData(b);
        if (!ca || !cb) return 1e9;
        double dx = cb->coordX - ca->coordX;
        double dy = cb->coordY - ca->coordY;
        return std::sqrt(dx * dx + dy * dy);
    }

    bool DriverAgentSystem::isNeighbor(const CityId& a, const CityId& b) const {
        return straightDistanceKm(a, b) <= NEIGHBOR_RADIUS_KM;
    }

    double DriverAgentSystem::travelMinutes(const CityId& a, const CityId& b, double speed) const {
        return straightDistanceKm(a, b) * 1.3 / std::max(1.0, speed) * 60.0;
    }

    void DriverAgentSystem::assignVehicle(const std::string& agentId,
        const std::string& vehicleInstanceId) {
        auto it = agents_.find(agentId);
        if (it == agents_.end()) return;

        it->second.vehicleRef = vehicleInstanceId;
        if (vehicles_) {
            double cap = vehicles_->getMarketCapacityKg(vehicleInstanceId);
            if (cap > 0.0) it->second.capacityKg = cap; // рыночная ёмкость теперь от тягача
        }
    }

} // namespace kotr::drivers