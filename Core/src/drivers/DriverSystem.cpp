#include "kotr/drivers/DriverSystem.hpp"
#include "kotr/licenses/LicenseSystem.hpp"
#include <algorithm>
#include <cmath>

namespace kotr::drivers {

    using namespace kotr::core;

    namespace {
        int clampInt(int v, int lo, int hi) { return std::clamp(v, lo, hi); }
        double clamp01(double v) { return std::clamp(v, 0.0, 1.0); }

        bool hasTag(const std::vector<std::string>& tags, const std::string& tag) {
            return std::find(tags.begin(), tags.end(), tag) != tags.end();
        }
    }

    DriverSystem::DriverSystem(EventBus& eventBus, TimeSystem& time)
        : eventBus_(eventBus), time_(time) {
        eventBus_.subscribe<GameDayElapsed>([this](const GameDayElapsed& e) {
            updateDay(e.time);
            });
        eventBus_.subscribe<GameWeekElapsed>([this](const GameWeekElapsed& e) {
            rotateMarket();
            });
    }

    // === Регистрация ===

    void DriverSystem::registerDriver(const DriverPersonaData& persona,
        DriverEmploymentStatus initialStatus) {
        DriverState state;
        state.driverId = persona.id;
        state.currentVehicleCapacityKg = persona.vehicleCapacityKg;

        state.status = initialStatus;
        if (initialStatus == DriverEmploymentStatus::FreeParked &&
            countByStatus(DriverEmploymentStatus::FreeParked) >= MAX_VISIBLE_CANDIDATES) {
            state.status = DriverEmploymentStatus::FreeIndependent;
        }

        personas_[persona.id] = persona;
        states_[persona.id] = state;
    }

    // === Рынок труда ===

    std::vector<DriverId> DriverSystem::getVisibleCandidates() const {
        std::vector<DriverId> result;
        for (const auto& [id, st] : states_) {
            if (st.status == DriverEmploymentStatus::FreeParked) {
                result.push_back(id);
            }
        }
        std::sort(result.begin(), result.end());
        return result;
    }

    void DriverSystem::rotateMarket() {
        auto visible = getVisibleCandidates();

        if (!visible.empty()) {
            auto& st = states_[visible.front()];
            st.status = DriverEmploymentStatus::FreeIndependent;
        }

        if (countByStatus(DriverEmploymentStatus::FreeParked) < MAX_VISIBLE_CANDIDATES) {
            std::vector<DriverId> independents;
            for (const auto& [id, st] : states_) {
                if (st.status == DriverEmploymentStatus::FreeIndependent) {
                    independents.push_back(id);
                }
            }
            std::sort(independents.begin(), independents.end());

            if (!independents.empty()) {
                states_[independents.front()].status = DriverEmploymentStatus::FreeParked;
            }
        }

        eventBus_.publish(LaborMarketRotatedEvent{ countByStatus(DriverEmploymentStatus::FreeParked) });
    }

    // === Найм ===

    DriverSystem::HireCostBreakdown DriverSystem::calcHireCost(const DriverId& id) const {
        const auto* persona = getPersona(id);
        if (!persona) return { 0, 0, 0 };

        return { persona->signingBonus, persona->vehicleBuyoutPrice,
                persona->signingBonus + persona->vehicleBuyoutPrice };
    }

    bool DriverSystem::canHireMoreDrivers(const std::string& employerId) const {
        if (!licenses_) return true; // если LicenseSystem не подключён, пропускаем проверку

        int currentHired = static_cast<int>(getHiredDrivers(employerId).size());
        return licenses_->getLicenseCount(employerId) >= currentHired + 1;
    }

    bool DriverSystem::hireDriver(const DriverId& id, const std::string& employerId) {
        auto it = states_.find(id);
        if (it == states_.end()) return false;

        // Найм только с парковки (по Driver Design §3)
        if (it->second.status != DriverEmploymentStatus::FreeParked) return false;

        // Проверка квоты лицензий работодателя
        if (!canHireMoreDrivers(employerId)) return false;

        it->second.status = (employerId == "player")
            ? DriverEmploymentStatus::HiredByPlayer
            : DriverEmploymentStatus::HiredByNamed;
        it->second.employerRef = employerId;

        auto cost = calcHireCost(id);
        eventBus_.publish(DriverHiredEvent{ id, employerId, cost.total,
                                           it->second.currentVehicleCapacityKg });
        return true;
    }

    bool DriverSystem::fireDriver(const DriverId& id) {
        auto it = states_.find(id);
        if (it == states_.end()) return false;
        if (it->second.employerRef.empty()) return false;
        if (it->second.status == DriverEmploymentStatus::OutOfAction) return false;

        it->second.employerRef = "";
        it->second.status = (countByStatus(DriverEmploymentStatus::FreeParked) < MAX_VISIBLE_CANDIDATES)
            ? DriverEmploymentStatus::FreeParked
            : DriverEmploymentStatus::FreeIndependent;

        eventBus_.publish(DriverFiredEvent{ id });
        return true;
    }

    // === Запросы ===

    const DriverState* DriverSystem::getDriverState(const DriverId& id) const {
        auto it = states_.find(id);
        return it != states_.end() ? &it->second : nullptr;
    }

    const DriverPersonaData* DriverSystem::getPersona(const DriverId& id) const {
        auto it = personas_.find(id);
        return it != personas_.end() ? &it->second : nullptr;
    }

    std::vector<DriverId> DriverSystem::getHiredDrivers() const {
        std::vector<DriverId> result;
        for (const auto& [id, st] : states_) {
            if (st.employerRef == "player") {
                result.push_back(id);
            }
        }
        std::sort(result.begin(), result.end());
        return result;
    }

    std::vector<DriverId> DriverSystem::getHiredDrivers(const std::string& employerId) const {
        std::vector<DriverId> result;
        for (const auto& [id, st] : states_) {
            if (st.employerRef == employerId) {
                result.push_back(id);
            }
        }
        std::sort(result.begin(), result.end());
        return result;
    }

    double DriverSystem::getHiredCapacityKg() const {
        double total = 0.0;
        for (const auto& [id, st] : states_) {
            if (st.employerRef == "player") {
                total += st.currentVehicleCapacityKg;
            }
        }
        return total;
    }

    // === Экономика ===

    Money DriverSystem::calcDailySalary(const DriverId& id, Money dailyRevenue) const {
        const auto* persona = getPersona(id);
        const auto* st = getDriverState(id);
        if (!persona || !st) return 0;

        switch (st->paymentPolicy) {
        case PaymentPolicy::Fixed:
            return persona->salaryExpectation;
        case PaymentPolicy::Percent:
            return static_cast<Money>(std::round(
                static_cast<double>(dailyRevenue) * persona->paymentPercent / 100.0));
        case PaymentPolicy::FixedPlusBonus:
            return static_cast<Money>(std::round(
                0.6 * static_cast<double>(persona->salaryExpectation) +
                0.05 * static_cast<double>(dailyRevenue)));
        case PaymentPolicy::BonusOnly:
            return static_cast<Money>(std::round(0.15 * static_cast<double>(dailyRevenue)));
        }
        return persona->salaryExpectation;
    }

    // === Риск ===

    double DriverSystem::calcBetrayalRisk(const DriverId& id, double cargoTemptation) const {
        const auto* persona = getPersona(id);
        const auto* st = getDriverState(id);
        if (!persona || !st) return 0.0;

        double lowLoyalty = (100.0 - st->loyalty) / 100.0;
        double fatigue = st->fatigue / 100.0;
        double stress = (100.0 - st->morale) / 100.0;
        double hiddenTrait = (hasTag(persona->hiddenTags, "Driver.Hidden.Traitor") ||
            hasTag(persona->hiddenTags, "Driver.Hidden.Thief")) ? 1.0 : 0.0;

        double risk = 0.35 * lowLoyalty
            + 0.15 * fatigue
            + 0.15 * stress
            + 0.15 * paymentDiscontent(*st)
            + 0.10 * hiddenTrait
            + 0.10 * cargoTemptation;

        return clamp01(risk);
    }

    double DriverSystem::paymentDiscontent(const DriverState& st) {
        switch (st.paymentPolicy) {
        case PaymentPolicy::BonusOnly:   return 0.35;
        case PaymentPolicy::Percent:     return 0.20;
        case PaymentPolicy::FixedPlusBonus: return 0.10;
        case PaymentPolicy::Fixed:       return 0.10;
        }
        return 0.10;
    }

    // === Симуляция ===

    void DriverSystem::updateDay(GameTime now) {
        // 1. Возврат водителей из OutOfAction
        for (auto& [id, st] : states_) {
            if (st.status == DriverEmploymentStatus::OutOfAction &&
                now.totalMinutes >= st.outOfActionUntil) {

                const auto* persona = getPersona(id);
                double ersatz = persona ? persona->vehicleCapacityKg * ERSATZ_CAPACITY_FACTOR : 0.0;
                st.currentVehicleCapacityKg = ersatz;
                st.status = (countByStatus(DriverEmploymentStatus::FreeParked) < MAX_VISIBLE_CANDIDATES)
                    ? DriverEmploymentStatus::FreeParked
                    : DriverEmploymentStatus::FreeIndependent;

                eventBus_.publish(DriverReturnedEvent{ id, ersatz });
            }
        }

        // 2. Ежедневное обновление нанятых водителей
        for (auto& [id, st] : states_) {
            if (st.employerRef.empty()) continue;

            if (st.status == DriverEmploymentStatus::Resting) {
                st.fatigue = clampInt(st.fatigue - 30, 0, 100);
                if (st.fatigue <= 20) {
                    st.status = DriverEmploymentStatus::HiredByPlayer;
                }
                continue;
            }

            if (st.status != DriverEmploymentStatus::HiredByPlayer &&
                st.status != DriverEmploymentStatus::HiredByNamed &&
                st.status != DriverEmploymentStatus::OnContract) {
                continue;
            }

            st.fatigue = clampInt(st.fatigue + 10, 0, 100);

            if (st.fatigue > 70) {
                st.morale = clampInt(st.morale - 5, 0, 100);
            }
            else {
                st.morale = clampInt(st.morale + 1, 0, 100);
            }

            if (st.morale < 30) {
                st.loyalty = clampInt(st.loyalty - 2, 0, 100);
            }
            else if (st.morale > 70) {
                st.loyalty = clampInt(st.loyalty + 1, 0, 100);
            }

            eventBus_.publish(DriverSalaryDueEvent{ id, st.employerRef, calcDailySalary(id, 0) });
        }
    }

    void DriverSystem::driverVehicleDestroyed(const DriverId& id, GameTime now) {
        auto it = states_.find(id);
        if (it == states_.end()) return;

        it->second.status = DriverEmploymentStatus::OutOfAction;
        it->second.outOfActionUntil = now.totalMinutes + OUT_OF_ACTION_DURATION;
        it->second.employerRef = "";
        it->second.currentVehicleCapacityKg = 0.0;

        eventBus_.publish(DriverVehicleDestroyedEvent{ id, it->second.outOfActionUntil });
    }

    void DriverSystem::sendToRest(const DriverId& id) {
        auto it = states_.find(id);
        if (it == states_.end()) return;
        if (it->second.employerRef == "player" &&
            it->second.status == DriverEmploymentStatus::HiredByPlayer) {
            it->second.status = DriverEmploymentStatus::Resting;
        }
    }

    int DriverSystem::countByStatus(DriverEmploymentStatus s) const {
        int count = 0;
        for (const auto& [id, st] : states_) {
            if (st.status == s) count++;
        }
        return count;
    }

} // namespace kotr::drivers