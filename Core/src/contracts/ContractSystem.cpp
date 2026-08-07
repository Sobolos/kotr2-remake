#include "kotr/contracts/ContractSystem.hpp"
#include "kotr/risk/RiskSystem.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>

namespace kotr::contracts {

    using namespace kotr::core;
    using namespace kotr::data;
    using namespace kotr::economy;
    using namespace kotr::risk;

    ContractSystem::ContractSystem(EventBus& eventBus, EconomySystem& economy, TimeSystem& time)
        : eventBus_(eventBus), economy_(economy), time_(time) {
        eventBus_.subscribe<GameHourElapsed>([this](const GameHourElapsed& e) {
            update(e.time);
            });
    }

    // === Симуляция ===

    void ContractSystem::update(GameTime now) {
        generateContracts(now);
        resolveDeliveries(now);
    }

    void ContractSystem::generateContracts(GameTime now) {
        int activeCount = 0;
        for (const auto& c : contracts_) if (c.active) activeCount++;
        if (activeCount >= MAX_ACTIVE_CONTRACTS) return;

        // Контракт = неудовлетворённый спрос (по исправленной модели)
        for (const auto& cityId : economy_.getAllCityIds()) {
            for (const auto& goodId : economy_.getAllGoodIds()) {
                if (activeCount >= MAX_ACTIVE_CONTRACTS) return;

                // Не дублируем активный контракт на ту же пару (назначение, товар)
                bool exists = false;
                for (const auto& c : contracts_) {
                    if (c.active && c.destination == cityId && c.good == goodId) { exists = true; break; }
                }
                if (exists) continue;

                double supply = economy_.getSupply(cityId, goodId);
                double demand = economy_.getDemand(cityId, goodId);
                double uncovered = demand - supply;
                if (uncovered < MIN_CONTRACT_TONS) continue;

                // Источник: город с наибольшим запасом этого товара
                CityId bestOrigin;
                double bestSupply = 0.0;
                for (const auto& otherId : economy_.getAllCityIds()) {
                    if (otherId == cityId) continue;
                    double s = economy_.getSupply(otherId, goodId);
                    if (s > bestSupply) { bestSupply = s; bestOrigin = otherId; }
                }
                if (bestOrigin.empty() || bestSupply < MIN_CONTRACT_TONS) continue;

                const auto* goodData = economy_.getGoodData(goodId);
                if (!goodData) continue;

                ContractOffer offer;
                std::ostringstream idStream;
                idStream << "ctr_" << std::setw(4) << std::setfill('0') << nextContractId_++;
                offer.id = idStream.str();
                offer.origin = bestOrigin;
                offer.destination = cityId;
                offer.good = goodId;
                offer.demandTons = std::min(uncovered, MAX_CONTRACT_TONS);
                offer.distanceKm = calcDistance(bestOrigin, cityId);
                offer.createdAt = now.totalMinutes;

                double deficitRatio = economy_.getDeficitRatio(cityId, goodId);
                if (deficitRatio >= 2.5)                    offer.type = ContractType::Urgent;
                else if (goodData->dangerModifier >= 0.5)   offer.type = ContractType::Dangerous;
                else if (!goodData->legal)                  offer.type = ContractType::Illegal;
                else                                        offer.type = ContractType::Normal;

                contracts_.push_back(offer);
                activeCount++;
                eventBus_.publish(ContractGeneratedEvent{ offer });
            }
        }
    }

    void ContractSystem::resolveDeliveries(GameTime now) {
        for (auto& c : contracts_) {
            for (auto& p : c.participants) {
                if (p.delivered || p.arrivalTime > now.totalMinutes) continue;

                p.delivered = true;
                c.deliveredTons += p.massTons;

                // Груз дошёл: экономика обновляется
                economy_.deliverGood(c.destination, c.good, p.massTons);

                // Оплата считается в момент доставки: опоздавшие получают меньше
                bool onTime = p.arrivalTime <= p.deadline;
                Money payout = calcPayout(c, p.massTons, now);
                if (!onTime) payout = static_cast<Money>(payout * 0.8); // штраф 20%

                eventBus_.publish(ContractDeliveryArrivedEvent{
                    c.id, p.carrier.id, p.carrier.kind, p.carrier.employerId,
                    p.massTons, c.distanceKm, payout, onTime });

                // Первая доставка: зачёт лицензии только Player/Named
                if (!c.firstDelivered) {
                    c.firstDelivered = true;
                    c.firstDelivererId = p.carrier.id;
                    c.firstDelivererEligible = (p.carrier.kind != CarrierKind::MassAI);
                    eventBus_.publish(ContractFirstDeliveryEvent{
                        c.id, p.carrier.id, p.carrier.kind, c.firstDelivererEligible });
                }
            }

            // Спрос покрыт → контракт деактивируется.
            // Те, кто ещё в пути, доезжают и остаются конкурентами.
            if (c.active && c.deliveredTons >= c.demandTons - 1e-9) {
                c.active = false;
                eventBus_.publish(ContractDeactivatedEvent{ c.id });
            }
        }
    }

    // === Присоединение ===

    bool ContractSystem::joinContract(const std::string& contractId,
        const CarrierRef& carrier, double massTons,
        GameMinutes startDelayMinutes) {
        ContractOffer* c = findContract(contractId);
        if (!c || !c->active) return false;
        if (massTons <= 0.0) return false;

        // Тягач должен тянуть массу
        if (carrier.capacityKg > 0.0 && massTons * 1000.0 > carrier.capacityKg) return false;

        // Нельзя взять больше остатка спроса (с учётом уже везущих)
        if (massTons > remainingTons(*c)) return false;

        ContractParticipant p;
        p.carrier = carrier;
        p.massTons = massTons;
        p.joinTime = time_.currentTime().totalMinutes;

        double travelMinutes = c->distanceKm / std::max(1.0, carrier.speedKmh) * 60.0;
        p.arrivalTime = p.joinTime + startDelayMinutes + static_cast<GameMinutes>(travelMinutes);
        p.deadline = p.joinTime +
            static_cast<GameMinutes>((startDelayMinutes + travelMinutes) * bufferFor(c->type));

        c->participants.push_back(p);
        eventBus_.publish(ContractJoinedEvent{ contractId, carrier.id, carrier.kind, massTons });
        return true;
    }

    // === Запросы ===

    std::vector<ContractOffer> ContractSystem::getAvailableContracts() const {
        std::vector<ContractOffer> result;
        for (const auto& c : contracts_) {
            if (c.active) result.push_back(c);
        }
        return result;
    }

    const ContractOffer* ContractSystem::getContract(const std::string& id) const {
        for (const auto& c : contracts_) {
            if (c.id == id) return &c;
        }
        return nullptr;
    }

    ContractOffer* ContractSystem::findContract(const std::string& id) {
        for (auto& c : contracts_) {
            if (c.id == id) return &c;
        }
        return nullptr;
    }

    double ContractSystem::inTransitTons(const ContractOffer& c) const {
        double total = 0.0;
        for (const auto& p : c.participants) {
            if (!p.delivered) total += p.massTons;
        }
        return total;
    }

    double ContractSystem::remainingTons(const ContractOffer& c) const {
        return std::max(0.0, c.demandTons - c.deliveredTons - inTransitTons(c));
    }

    // === Расчёты ===

    double ContractSystem::calcDistance(const CityId& from, const CityId& to) const {
        const auto* f = economy_.getCityData(from);
        const auto* t = economy_.getCityData(to);
        if (!f || !t) return 0.0;
        double dx = t->coordX - f->coordX;
        double dy = t->coordY - f->coordY;
        return std::sqrt(dx * dx + dy * dy) * 1.3; // извилистость дорог
    }

    double ContractSystem::calcRouteRisk(const CityId& from, const CityId& to) const {
        const auto* f = economy_.getCityData(from);
        const auto* t = economy_.getCityData(to);
        double baseRisk = (f && t) ? (f->baseDanger + t->baseDanger) / 2.0 : 0.3;

        if (riskSystem_) {
            baseRisk += (riskSystem_->getPoliceHeat() / 100.0) * 0.25;
            baseRisk += (riskSystem_->getBanditPressure() / 100.0) * 0.25;
        }
        return std::min(2.0, baseRisk);
    }

    Money ContractSystem::calcPayout(const ContractOffer& c, double massTons, GameTime now) const {
        const auto* good = economy_.getGoodData(c.good);
        if (!good) return 0;

        // Формула из Economy Design §11: оплата считается НА МОМЕНТ ДОСТАВКИ.
        // Опоздавшие к покрытому спросу получают худший фактор — это и есть гонка.
        double deficit = economy_.getDeficitRatio(c.destination, c.good);
        double demandPriceFactor = std::clamp(0.7 + 0.3 * deficit, 0.7, 2.0);
        double riskFactor = 1.0 + 0.35 * calcRouteRisk(c.origin, c.destination)
            + 0.25 * good->dangerModifier;

        // competition_factor (0.85–1.15) по Economy Design §11.7
        // Если на маршруте много участников, оплата падает
        int participantCount = static_cast<int>(c.participants.size());
        double competitionFactor = 1.0;
        if (participantCount > 3) {
            competitionFactor = std::max(0.85, 1.0 - (participantCount - 3) * 0.05);
        }
        else if (participantCount == 1) {
            competitionFactor = 1.15; // монопольная выплата
        }

        double illegalFactor = good->legal ? 1.0 : 1.5;

        double payout = static_cast<double>(good->baseTariffPerTonKm)
            * massTons * c.distanceKm
            * demandPriceFactor
            * urgencyFactorFor(c.type)
            * riskFactor
            * illegalFactor
            * competitionFactor;

        return static_cast<Money>(std::round(payout));
    }

    double ContractSystem::bufferFor(ContractType type) {
        switch (type) {
        case ContractType::Urgent:    return 1.1;
        case ContractType::Dangerous: return 2.0;
        case ContractType::Illegal:   return 1.8;
        default:                      return 1.6;
        }
    }

    double ContractSystem::urgencyFactorFor(ContractType type) {
        switch (type) {
        case ContractType::Urgent:    return 1.5;
        case ContractType::Dangerous: return 1.25;
        case ContractType::Illegal:   return 1.3;
        case ContractType::Special:   return 2.0;
        default:                      return 1.0;
        }
    }

} // namespace kotr::contracts