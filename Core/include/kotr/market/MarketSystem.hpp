#pragma once
#include <string>
#include <unordered_map>
#include <kotr/core/Types.hpp>
#include <kotr/core/EventBus.hpp>

namespace kotr::market {

    // === События рынка ===

    struct MarketShareChangedEvent {
        double newSharePercent;
        double oldSharePercent;
    };

    struct VictoryConditionReachedEvent {
        double finalSharePercent;
    };

    struct CompetitorRemovedEvent {
        std::string competitorId;
    };

    /// Система рынка.
    /// Отслеживает долю рынка игрока по грузоподъёмности (по HL-GDD и DMS).
    /// Победа при достижении >51%.
    class MarketSystem {
    public:
        explicit MarketSystem(kotr::core::EventBus& eventBus);

        // --- Ёмкость игрока ---
        void setPlayerCapacity(double capacityKg);
        void adjustPlayerCapacity(double deltaKg);
        [[nodiscard]] double playerCapacity() const { return playerCapacityKg_; }

        // --- Ёмкость конкурентов ---
        void setCompetitorCapacity(const std::string& competitorId, double capacityKg);
        void removeCompetitor(const std::string& competitorId);
        [[nodiscard]] size_t competitorCount() const { return competitorCapacities_.size(); }

        // --- Доля рынка ---
        /// MarketShare = PlayerCapacity / (PlayerCapacity + CompetitorCapacity) * 100
        [[nodiscard]] double getMarketShare() const;
        [[nodiscard]] double getTotalMarketCapacity() const;
        [[nodiscard]] double getCompetitorCapacity() const;

        // --- Победа ---
        [[nodiscard]] bool isVictoryReached() const;
        void setVictoryThreshold(double threshold) { victoryThreshold_ = threshold; }
        [[nodiscard]] double victoryThreshold() const { return victoryThreshold_; }

    private:
        void recalculateMarketShare();

        kotr::core::EventBus& eventBus_;
        double playerCapacityKg_ = 0.0;
        std::unordered_map<std::string, double> competitorCapacities_;
        double lastMarketShare_ = 0.0;
        double victoryThreshold_ = 51.0; // >51% для победы (по HL-GDD)
    };

} // namespace kotr::market