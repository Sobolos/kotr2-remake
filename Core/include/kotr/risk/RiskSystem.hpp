#pragma once
#include <string>
#include <random>
#include <kotr/core/Types.hpp>
#include <kotr/core/EventBus.hpp>
#include <kotr/core/TimeSystem.hpp>

namespace kotr::risk {
    struct ThreatLevelChangedEvent {
        int newThreatLevel; // -1 (Safe), 0 (Low), 1 (Medium), 2 (High), 3 (Extreme)
        std::string threatName;
    };

    struct ThreatMeter {
        int policeHeat = 0;      // 0..100
        int banditPressure = 0;  // 0..100
        [[nodiscard]] int getOverallThreat() const { return std::max(policeHeat, banditPressure); }
    };

    class RiskSystem {
    public:
        explicit RiskSystem(core::EventBus& eventBus, core::TimeSystem& time);
        void updateHour(core::GameTime now);

        void addPoliceHeat(int amount);
        void addBanditPressure(int amount);
        void setPoliceHeat(int amount);
        void setBanditPressure(int amount);

        [[nodiscard]] int getPoliceHeat() const;
        [[nodiscard]] int getBanditPressure() const;
        [[nodiscard]] int getCurrentThreatLevel() const;
        [[nodiscard]] double getRouteRiskMultiplier() const;

        static constexpr int THRESHOLD_LOW = 15;
        static constexpr int THRESHOLD_MEDIUM = 35;
        static constexpr int THRESHOLD_HIGH = 60;
        static constexpr int THRESHOLD_EXTREME = 90;

        void setSeed(uint64_t seed) { rng_.seed(static_cast<std::mt19937::result_type>(seed)); }

    private:
        void decayHeat();
        void evaluateThreatLevel();

        core::EventBus& eventBus_;
        core::TimeSystem& time_;
        ThreatMeter meter_;
        int currentThreatLevel_ = -1;
        std::mt19937 rng_{ 42 };
    };
}