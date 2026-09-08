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
    /// 
    /// По Economy Design §16.4: TotalMarketCapacity = сумма грузоподъёмности ВСЕХ 52 тягачей
    /// (игрок + 51 именной водитель). Уничтоженные машины конкурентов не уменьшают общую ёмкость
    /// (водитель получает Ersatz-тягач), поэтому общая ёмкость рынка фиксирована.
    class MarketSystem {
    public:
        // Суммарная грузоподъёмность всех 52 тягачей (игрок + 51 именной водитель)
        // Среднее значение: ~10000 кг на тягач = 520000 кг (настраивается через данные)
        static constexpr double DEFAULT_TOTAL_MARKET_CAPACITY_KG = 520000.0;

        explicit MarketSystem(kotr::core::EventBus& eventBus,
            double totalMarketCapacityKg = DEFAULT_TOTAL_MARKET_CAPACITY_KG);

        // --- Ёмкость игрока ---
        void setPlayerCapacity(double capacityKg);
        void adjustPlayerCapacity(double deltaKg);
        [[nodiscard]] double playerCapacity() const { return playerCapacityKg_; }

        // --- Ёмкость конкурентов ---
        // Примечание: competitorCapacities используется только для отслеживания ёмкости конкурентов,
        // но НЕ влияет на getTotalMarketCapacity() (по Economy Design §16.4)
        void setCompetitorCapacity(const std::string& competitorId, double capacityKg);
        void removeCompetitor(const std::string& competitorId);
        [[nodiscard]] size_t competitorCount() const { return competitorCapacities_.size(); }

        // --- Доля рынка ---
        /// MarketShare = PlayerCapacity / TotalMarketCapacity * 100
        /// По Economy Design §16.4: TotalMarketCapacity фиксирована (все 52 тягача)
        [[nodiscard]] double getMarketShare() const;
        [[nodiscard]] double getTotalMarketCapacity() const { return totalMarketCapacityKg_; }
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
        double totalMarketCapacityKg_;  // фиксированная общая ёмкость (все 52 тягача)
        double lastMarketShare_ = 0.0;
        double victoryThreshold_ = 51.0; // >51% для победы (по HL-GDD)
    };

} // namespace kotr::market