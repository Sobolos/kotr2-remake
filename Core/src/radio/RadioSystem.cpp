#include "kotr/radio/RadioSystem.hpp"

namespace kotr::radio {
    using namespace kotr::core;

    RadioSystem::RadioSystem(EventBus& eventBus) : eventBus_(eventBus) {
        eventBus_.subscribe<risk::ThreatLevelChangedEvent>(
            [this](const risk::ThreatLevelChangedEvent& e) { onThreatChanged(e); });
    }

    void RadioSystem::playAnnouncement(const std::string& messageKey, int priority) {
        eventBus_.publish(RadioAnnouncementEvent{ "news", messageKey, priority });
    }

    void RadioSystem::onThreatChanged(const risk::ThreatLevelChangedEvent& e) {
        if (e.newThreatLevel >= 2) playAnnouncement("radio.danger.extreme", 5);
        else if (e.newThreatLevel == 1) playAnnouncement("radio.danger.medium", 3);
        else if (e.newThreatLevel == 0) playAnnouncement("radio.danger.low", 1);
    }
}