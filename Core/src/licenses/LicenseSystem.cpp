#include "kotr/licenses/LicenseSystem.hpp"
#include <algorithm>

namespace kotr::licenses {
    using namespace kotr::core;

    LicenseSystem::LicenseSystem(EventBus& eventBus) : eventBus_(eventBus) {
        eventBus_.subscribe<leaderboard::LicenseAwardedEvent>(
            [this](const leaderboard::LicenseAwardedEvent& e) { onLicenseAwarded(e); });
    }

    void LicenseSystem::awardLicenses(const std::string& ownerRef, int count) {
        if (count > 0) ownerLicenses_[ownerRef] += count;
    }

    void LicenseSystem::revokeLicenses(const std::string& ownerRef, int count) {
        if (count > 0) {
            int& current = ownerLicenses_[ownerRef];
            current = std::max(0, current - count);
        }
    }

    bool LicenseSystem::hasLicenses(const std::string& ownerRef, int requiredCount) const {
        auto it = ownerLicenses_.find(ownerRef);
        return (it != ownerLicenses_.end() ? it->second : 0) >= requiredCount;
    }

    int LicenseSystem::getLicenseCount(const std::string& ownerRef) const {
        auto it = ownerLicenses_.find(ownerRef);
        return (it != ownerLicenses_.end()) ? it->second : 0;
    }

    void LicenseSystem::onLicenseAwarded(const leaderboard::LicenseAwardedEvent& e) {
        awardLicenses(e.winnerId, e.licenseCount);
    }
}