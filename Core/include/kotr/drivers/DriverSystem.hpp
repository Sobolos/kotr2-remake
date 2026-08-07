#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <kotr/core/Types.hpp>
#include <kotr/core/EventBus.hpp>
#include <kotr/core/TimeSystem.hpp>

namespace kotr::licenses { class LicenseSystem; }

namespace kotr::drivers {

    // Статус занятости (по DMS + жизненный цикл из Driver Design)
    enum class DriverEmploymentStatus {
        FreeParked,       // на парковке, видимый кандидат
        FreeIndependent,  // свободен, берёт заказы, не виден на рынке
        HiredByPlayer,
        HiredByNamed,
        OnContract,
        Resting,
        Repairing,
        Arrested,
        Incapacitated,
        OutOfAction,      // после уничтожения машины, 1 игровой день
        Betraying,
        RaceEvent
    };

    // Политика оплаты (по Economy Design §15.2)
    enum class PaymentPolicy {
        Fixed,          // фикс в день
        Percent,        // процент от выручки
        FixedPlusBonus, // оклад + бонус
        BonusOnly       // только бонус
    };

    /// Статичный портрет водителя (по DMS: DriverPersonaData)
    struct DriverPersonaData {
        core::DriverId id;                    // "drv_kobra"
        std::string nameKey;                  // "loc.driver.kobra.name"
        core::CityId homeCity;
        std::vector<std::string> personalityTags; // "Driver.Personality.Calm"
        std::vector<std::string> hiddenTags;      // "Driver.Hidden.Traitor"

        int drivingSkill = 50;      // 0–100
        int repairSkill = 50;
        int negotiationSkill = 50;

        std::vector<std::string> requiredLicenses; // 0–3 лицензии
        core::Money salaryExpectation = 150;       // базовая ставка в день
        int paymentPercent = 10;                   // для политики Percent
        double vehicleCapacityKg = 10000;          // рыночная ёмкость его тягача
        core::Money vehicleBuyoutPrice = 25000;    // выкуп при найме
        core::Money signingBonus = 500;            // подъёмные
    };

    /// Рантайм-состояние водителя (по DMS: DriverState)
    struct DriverState {
        core::DriverId driverId;
        DriverEmploymentStatus status = DriverEmploymentStatus::FreeParked;
        std::string employerRef;              // "player", если нанят
        int loyalty = 50;                     // 0–100
        int morale = 50;                      // 0–100
        int fatigue = 0;                      // 0–100
        PaymentPolicy paymentPolicy = PaymentPolicy::Fixed;
        core::GameMinutes outOfActionUntil = 0;
        double currentVehicleCapacityKg = 0;  // текущая (может быть Ersatz)
    };

    // === События ===

    struct DriverHiredEvent {
        core::DriverId driverId;
        std::string employerId;      // "player" или id именного
        core::Money totalCost;
        double capacityKg;
    };

    struct DriverFiredEvent {
        core::DriverId driverId;
    };

    struct DriverSalaryDueEvent {
        core::DriverId driverId;
        std::string employerId;
        core::Money amount;
    };

    struct DriverVehicleDestroyedEvent {
        core::DriverId driverId;
        core::GameMinutes returnTime;
    };

    struct DriverReturnedEvent {
        core::DriverId driverId;
        double ersatzCapacityKg;
    };

    struct LaborMarketRotatedEvent {
        int visibleCount;
    };

    /// Система водителей.
    /// Рынок труда, найм, зарплаты, лояльность, риск предательства, Ersatz.
    class DriverSystem {
    public:
        static constexpr int MAX_VISIBLE_CANDIDATES = 10;                 // по Driver Design: 8–14
        static constexpr core::GameMinutes OUT_OF_ACTION_DURATION = 60 * 24; // 1 игровой день
        static constexpr double ERSATZ_CAPACITY_FACTOR = 0.5;             // Ersatz не лучше уничтоженной

        DriverSystem(core::EventBus& eventBus, core::TimeSystem& time);


        kotr::licenses::LicenseSystem* licenses_ = nullptr;
        void setLicenseSystem(kotr::licenses::LicenseSystem* ls) { licenses_ = ls; }

        // --- Регистрация данных ---
        void registerDriver(const DriverPersonaData& persona,
            DriverEmploymentStatus initialStatus = DriverEmploymentStatus::FreeParked);

        // --- Рынок труда ---
        [[nodiscard]] std::vector<core::DriverId> getVisibleCandidates() const;
        void rotateMarket();

        // --- Найм ---
        struct HireCostBreakdown {
            core::Money signingBonus;
            core::Money vehicleBuyout;
            core::Money total;
        };
        [[nodiscard]] HireCostBreakdown calcHireCost(const core::DriverId& id) const;
        [[nodiscard]] bool hasRequiredLicenses(const core::DriverId& id) const;
        bool hireDriver(const core::DriverId& id, const std::string& employerId = "player");
        bool fireDriver(const core::DriverId& id);
        [[nodiscard]] std::vector<core::DriverId> getHiredDrivers(const std::string& employerId) const;

        // --- Лицензии игрока ---
        void addPlayerLicense(const std::string& license);

        // --- Запросы ---
        [[nodiscard]] const DriverState* getDriverState(const core::DriverId& id) const;
        [[nodiscard]] const DriverPersonaData* getPersona(const core::DriverId& id) const;
        [[nodiscard]] std::vector<core::DriverId> getHiredDrivers() const;
        [[nodiscard]] double getHiredCapacityKg() const;

        // --- Экономика ---
        [[nodiscard]] core::Money calcDailySalary(const core::DriverId& id,
            core::Money dailyRevenue = 0) const;

        // --- Риск ---
        [[nodiscard]] double calcBetrayalRisk(const core::DriverId& id,
            double cargoTemptation = 0.0) const;

        // --- Симуляция ---
        void updateDay(core::GameTime now);
        void driverVehicleDestroyed(const core::DriverId& id, core::GameTime now);
        void sendToRest(const core::DriverId& id);

    private:
        [[nodiscard]] int countByStatus(DriverEmploymentStatus s) const;
        [[nodiscard]] static double paymentDiscontent(const DriverState& st);

        core::EventBus& eventBus_;
        core::TimeSystem& time_;
        std::unordered_map<core::DriverId, DriverPersonaData> personas_;
        std::unordered_map<core::DriverId, DriverState> states_;
        std::vector<std::string> playerLicenses_;
    };

} // namespace kotr::drivers