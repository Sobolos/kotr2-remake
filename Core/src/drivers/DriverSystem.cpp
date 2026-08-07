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
        // Автообновление: каждый игровой день и каждая неделя (по TAD)
        eventBus_.subscribe<GameDayElapsed>([this](const GameDayElapsed& e) {
            updateDay(e.time);
            });
        eventBus_.subscribe<GameWeekElapsed>([this](const GameWeekElapsed&) {
            rotateMarket();
            });
    }

    // === Регистрация ===

    void DriverSystem::registerDriver(const DriverPersonaData& persona,
        DriverEmploymentStatus initialStatus) {
        DriverState state;
        state.driverId = persona.id;
        state.currentVehicleCapacityKg = persona.vehicleCapacityKg;

        // Если парковка заполнена — кандидат уходит в независимые
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
        std::sort(result.begin(), result.end()); // детерминированный порядок
        return result;
    }

    void DriverSystem::rotateMarket() {
        // Еженедельная ротация (по Driver Design §3):
        // один кандидат с парковки уходит в независимые,
        // один независимый занимает слот, если есть место.
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

        // HireCost = signing_bonus + vehicle_buyout (по Economy Design §15.1)
        return { persona->signingBonus, persona->vehicleBuyoutPrice,
                persona->signingBonus + persona->vehicleBuyoutPrice };
    }

    bool DriverSystem::hasRequiredLicenses(const DriverId& id) const {
        const auto* persona = getPersona(id);
        if (!persona) return false;

        if (!licenses_) return true; // если LicenseSystem не подключён, пропускаем проверку

        for (const auto& req : persona->requiredLicenses) {
            if (!licenses_->hasLicense("player", req)) {
                return false;
            }
        }
        return true;
    }

    bool DriverSystem::hireDriver(const DriverId& id, const std::string& employerId) {
        auto it = states_.find(id);
        if (it == states_.end()) return false;

        // Найм только с парковки (по Driver Design §3)
        if (it->second.status != DriverEmploymentStatus::FreeParked) return false;
        if (!hasRequiredLicenses(id)) return false;

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

    void DriverSystem::addPlayerLicense(const std::string& license) {
        if (std::find(playerLicenses_.begin(), playerLicenses_.end(), license) == playerLicenses_.end()) {
            playerLicenses_.push_back(license);
        }
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

        // Политики оплаты (по Economy Design §15.2)
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

        // Формула из Risk System Design §14.2:
        // BetrayalRisk = 0.35*LowLoyalty + 0.15*Fatigue + 0.15*Stress
        //              + 0.15*PaymentDiscontent + 0.10*HiddenTrait + 0.10*CargoTemptation
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
        // Нестабильный доход повышает недовольство (стартовые ориентиры)
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
        // 1. Возврат водителей из OutOfAction (по Driver Design §2)
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
                // Отдых снижает усталость
                st.fatigue = clampInt(st.fatigue - 30, 0, 100);
                if (st.fatigue <= 20) {
                    st.status = DriverEmploymentStatus::HiredByPlayer;
                }
                continue; // В отдыхе зарплата не начисляется
            }

            if (st.status != DriverEmploymentStatus::HiredByPlayer &&
                st.status != DriverEmploymentStatus::HiredByNamed &&
                st.status != DriverEmploymentStatus::OnContract) {
                continue;
            }

            // Усталость растёт от работы
            st.fatigue = clampInt(st.fatigue + 10, 0, 100);

            // Мораль и лояльность (по Economy Design §15.4)
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

            // Зарплата за день
            eventBus_.publish(DriverSalaryDueEvent{ id, st.employerRef, calcDailySalary(id, 0) });
        }
    }

    void DriverSystem::driverVehicleDestroyed(const DriverId& id, GameTime now) {
        auto it = states_.find(id);
        if (it == states_.end()) return;

        // По Driver Design §2: контракт провален, трудовое соглашение
        // расторгается БЕЗ штрафа, водитель OutOfAction на 1 день.
        it->second.status = DriverEmploymentStatus::OutOfAction;
        it->second.outOfActionUntil = now.totalMinutes + OUT_OF_ACTION_DURATION;
        it->second.employerRef = "";
        it->second.currentVehicleCapacityKg = 0.0; // машина уничтожена

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

    std::vector<DriverId> DriverSystem::getHiredDrivers(const std::string& employerId) const {
        std::vector<DriverId> result;
        for (const auto& [id, st] : states_) {
            if (st.employerRef == employerId) result.push_back(id);
        }
        std::sort(result.begin(), result.end());
        return result;
    }

} // namespace kotr::drivers