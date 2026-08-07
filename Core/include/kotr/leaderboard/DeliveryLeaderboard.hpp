#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <random>
#include <kotr/core/Types.hpp>
#include <kotr/core/EventBus.hpp>
#include <kotr/core/TimeSystem.hpp>

namespace kotr::leaderboard {

    // Категория рейтинга (по Driver Design §4)
    enum class LeaderboardCategory {
        Food, Timber, Coal, Fuel, Consumer, Metal, Ore, Valuables, Illegal, Overall
    };

    /// Запись в рейтинге
    struct LeaderboardEntry {
        std::string carrierId;
        bool isNamed = false;           // player/named eligible для лицензии
        double score = 0.0;
        int deliveries = 0;
    };

    /// Рейтинг по категории за период
    struct CategoryLeaderboard {
        LeaderboardCategory category;
        std::vector<LeaderboardEntry> entries; // отсортированы по score (desc)
    };

    // === События ===

    struct LicenseAwardedEvent {
        std::string winnerId;
        bool isNamed;
        std::string licenseCategory;    // "License.Food", "License.Overall"
        double score;
    };

    /// Система рейтингов доставки.
    /// Накапливает score за доставки, определяет победителей каждые 7 дней.
    /// По Driver Design §4: лицензии выдаются за первые места.
    class DeliveryLeaderboard {
    public:
        static constexpr int PERIOD_DAYS = 7;

        explicit DeliveryLeaderboard(core::EventBus& eventBus, core::TimeSystem& time);

        // --- Запросы ---
        [[nodiscard]] CategoryLeaderboard getLeaderboard(LeaderboardCategory cat) const;
        [[nodiscard]] const LeaderboardEntry* getEntry(LeaderboardCategory cat,
            const std::string& carrierId) const;
        [[nodiscard]] std::vector<std::string> getTopCarriers(LeaderboardCategory cat, int count = 3) const;

        // --- Симуляция ---
        void evaluatePeriod();  // вызывается вручную или по GameWeekElapsed

        void setSeed(uint64_t seed) { rng_.seed(static_cast<std::mt19937::result_type>(seed)); }

    private:
        void onDelivery(const contracts::ContractDeliveryArrivedEvent& e);
        void onWeekElapsed(const core::GameWeekElapsed& e);

        [[nodiscard]] static LeaderboardCategory goodToCategory(const core::GoodId& goodId);
        [[nodiscard]] static double categoryMultiplier(LeaderboardCategory cat);
        [[nodiscard]] static std::string categoryToLicenseKey(LeaderboardCategory cat);

        core::EventBus& eventBus_;
        core::TimeSystem& time_;

        // score за текущий период: category -> carrierId -> score
        std::unordered_map<LeaderboardCategory,
            std::unordered_map<std::string, LeaderboardEntry>> periodScores_;

        // carrierId -> isNamed (кэш для eligibility)
        std::unordered_map<std::string, bool> carrierIsNamed_;

        std::mt19937 rng_{ 42 };
        int periodCount_ = 0;
    };

} // namespace kotr::leaderboard