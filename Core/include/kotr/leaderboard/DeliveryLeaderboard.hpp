#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <random>
#include <kotr/core/Types.hpp>
#include <kotr/core/EventBus.hpp>
#include <kotr/core/TimeSystem.hpp>
#include <kotr/contracts/ContractSystem.hpp>

namespace kotr::leaderboard {

    struct LeaderboardEntry {
        std::string carrierId;
        bool isNamed = false;
        double score = 0.0;
        int deliveries = 0;
    };

    struct LeaderboardData {
        std::vector<LeaderboardEntry> entries;
    };

    struct LicenseAwardedEvent {
        std::string winnerId;
        bool isNamed;
        int licenseCount;
        double score;
    };

    class DeliveryLeaderboard {
    public:
        static constexpr int PERIOD_DAYS = 7;

        explicit DeliveryLeaderboard(core::EventBus& eventBus, core::TimeSystem& time);

        [[nodiscard]] LeaderboardData getLeaderboard() const;
        [[nodiscard]] const LeaderboardEntry* getEntry(const std::string& carrierId) const;
        [[nodiscard]] std::vector<std::string> getTopCarriers(int count = 3) const;

        void evaluatePeriod();
        void setSeed(uint64_t seed) { rng_.seed(static_cast<std::mt19937::result_type>(seed)); }

    private:
        void onDelivery(const contracts::ContractDeliveryArrivedEvent& e);
        void onWeekElapsed(const core::GameWeekElapsed& e);

        core::EventBus& eventBus_;
        core::TimeSystem& time_;

        std::unordered_map<std::string, LeaderboardEntry> periodScores_;
        std::unordered_map<std::string, bool> carrierIsNamed_;

        // Счётчик серий для выдачи нескольких лицензий
        std::unordered_map<std::string, int> carrierStreak_;
        std::string lastWeekWinnerId_;

        std::mt19937 rng_{ 42 };
        int periodCount_ = 0;
    };

} // namespace kotr::leaderboard