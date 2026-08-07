#pragma once
#include <string>
#include <unordered_map>
#include <kotr/core/Types.hpp>
#include <kotr/core/EventBus.hpp>
#include <kotr/leaderboard/DeliveryLeaderboard.hpp>

namespace kotr::licenses {
    class LicenseSystem {
    public:
        explicit LicenseSystem(core::EventBus& eventBus);

        void awardLicenses(const std::string& ownerRef, int count);
        void revokeLicenses(const std::string& ownerRef, int count);

        [[nodiscard]] bool hasLicenses(const std::string& ownerRef, int requiredCount) const;
        [[nodiscard]] int getLicenseCount(const std::string& ownerRef) const;

    private:
        void onLicenseAwarded(const leaderboard::LicenseAwardedEvent& e);
        core::EventBus& eventBus_;
        std::unordered_map<std::string, int> ownerLicenses_;
    };
}