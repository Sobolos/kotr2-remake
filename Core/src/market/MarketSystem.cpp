#include "kotr/market/MarketSystem.hpp"
#include <cmath>

namespace kotr::market {

    using namespace kotr::core;

    MarketSystem::MarketSystem(EventBus& eventBus)
        : eventBus_(eventBus) {}

    void MarketSystem::setPlayerCapacity(double capacityKg) {
        playerCapacityKg_ = capacityKg;
        recalculateMarketShare();
    }

    void MarketSystem::adjustPlayerCapacity(double deltaKg) {
        playerCapacityKg_ = std::max(0.0, playerCapacityKg_ + deltaKg);
        recalculateMarketShare();
    }

    void MarketSystem::setCompetitorCapacity(const std::string& competitorId, double capacityKg) {
        competitorCapacities_[competitorId] = capacityKg;
        recalculateMarketShare();
    }

    void MarketSystem::removeCompetitor(const std::string& competitorId) {
        auto it = competitorCapacities_.find(competitorId);
        if (it != competitorCapacities_.end()) {
            competitorCapacities_.erase(it);
            eventBus_.publish(CompetitorRemovedEvent{ competitorId });
            recalculateMarketShare();
        }
    }

    double MarketSystem::getMarketShare() const {
        double total = getTotalMarketCapacity();
        if (total <= 0.0) return 0.0;
        return (playerCapacityKg_ / total) * 100.0;
    }

    double MarketSystem::getTotalMarketCapacity() const {
        double competitorTotal = getCompetitorCapacity();
        return playerCapacityKg_ + competitorTotal;
    }

    double MarketSystem::getCompetitorCapacity() const {
        double total = 0.0;
        for (const auto& [id, capacity] : competitorCapacities_) {
            total += capacity;
        }
        return total;
    }

    bool MarketSystem::isVictoryReached() const {
        return getMarketShare() > victoryThreshold_;
    }

    void MarketSystem::recalculateMarketShare() {
        double newShare = getMarketShare();
        double oldShare = lastMarketShare_;

        if (std::abs(newShare - oldShare) > 0.01) {
            lastMarketShare_ = newShare;
            eventBus_.publish(MarketShareChangedEvent{ newShare, oldShare });

            // Проверяем условие победы
            if (newShare > victoryThreshold_ && oldShare <= victoryThreshold_) {
                eventBus_.publish(VictoryConditionReachedEvent{ newShare });
            }
        }
    }

} // namespace kotr::market