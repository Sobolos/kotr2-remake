#pragma once
#include <string>
#include <kotr/core/Types.hpp>
#include <kotr/core/EventBus.hpp>
#include <kotr/risk/RiskSystem.hpp>

namespace kotr::radio {
    struct RadioAnnouncementEvent {
        std::string channel;
        std::string messageKey;
        int priority;
    };

    class RadioSystem {
    public:
        explicit RadioSystem(core::EventBus& eventBus);
        void playAnnouncement(const std::string& messageKey, int priority = 1);

    private:
        void onThreatChanged(const risk::ThreatLevelChangedEvent& e);
        core::EventBus& eventBus_;
    };
}