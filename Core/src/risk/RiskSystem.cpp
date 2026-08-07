#include "kotr/risk/RiskSystem.hpp"
#include <algorithm>

namespace kotr::risk {
    using namespace kotr::core;

    RiskSystem::RiskSystem(EventBus& eventBus, TimeSystem& time) : eventBus_(eventBus), time_(time) {
        eventBus_.subscribe<GameHourElapsed>([this](const GameHourElapsed& e) { updateHour(e.time); });
    }

    void RiskSystem::updateHour(GameTime now) {
        decayHeat();
        evaluateThreatLevel();
    }

    void RiskSystem::decayHeat() {
        if (meter_.policeHeat > 0) meter_.policeHeat = std::max(0, meter_.policeHeat - 5); // Быстрый распад
        if (meter_.banditPressure > 0) meter_.banditPressure = std::max(0, meter_.banditPressure - 2);
    }

    void RiskSystem::evaluateThreatLevel() {
        int threat = meter_.getOverallThreat();
        int newLevel = -1;
        std::string threatName = "Safe";

        if (threat >= THRESHOLD_EXTREME) { newLevel = 3; threatName = "Extreme"; }
        else if (threat >= THRESHOLD_HIGH) { newLevel = 2; threatName = "High"; }
        else if (threat >= THRESHOLD_MEDIUM) { newLevel = 1; threatName = "Medium"; }
        else if (threat >= THRESHOLD_LOW) { newLevel = 0; threatName = "Low"; }

        if (newLevel != currentThreatLevel_) {
            currentThreatLevel_ = newLevel;
            eventBus_.publish(ThreatLevelChangedEvent{ currentThreatLevel_, threatName });
        }
    }

    void RiskSystem::addPoliceHeat(int amount) { meter_.policeHeat = std::clamp(meter_.policeHeat + amount, 0, 100); evaluateThreatLevel(); }
    void RiskSystem::addBanditPressure(int amount) { meter_.banditPressure = std::clamp(meter_.banditPressure + amount, 0, 100); evaluateThreatLevel(); }
    void RiskSystem::setPoliceHeat(int amount) { meter_.policeHeat = std::clamp(amount, 0, 100); evaluateThreatLevel(); }
    void RiskSystem::setBanditPressure(int amount) { meter_.banditPressure = std::clamp(amount, 0, 100); evaluateThreatLevel(); }

    int RiskSystem::getPoliceHeat() const { return meter_.policeHeat; }
    int RiskSystem::getBanditPressure() const { return meter_.banditPressure; }
    int RiskSystem::getCurrentThreatLevel() const { return currentThreatLevel_; }

    double RiskSystem::getRouteRiskMultiplier() const {
        return 1.0 + (static_cast<double>(meter_.getOverallThreat()) / 100.0) * 0.35;
    }
}