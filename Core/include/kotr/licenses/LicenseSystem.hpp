#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <kotr/core/Types.hpp>
#include <kotr/core/EventBus.hpp>
#include <kotr/leaderboard/DeliveryLeaderboard.hpp>

namespace kotr::licenses {

    /// Лицензия (по DMS: LicenseState)
    struct License {
        std::string licenseId;        // "License.Food"
        std::string ownerRef;         // "player", "drv_..."
        std::string category;         // "Food", "Overall"
        int sourceRanking = 1;        // место в рейтинге
    };

    /// Система лицензий.
    /// Хранит выданные лицензии, проверяет наличие при найме.
    class LicenseSystem {
    public:
        explicit LicenseSystem(core::EventBus& eventBus);

        // --- Управление ---
        void awardLicense(const std::string& ownerRef, const std::string& licenseId, int ranking = 1);
        void revokeLicense(const std::string& ownerRef, const std::string& licenseId);

        // --- Запросы ---
        [[nodiscard]] bool hasLicense(const std::string& ownerRef, const std::string& licenseId) const;
        [[nodiscard]] std::vector<std::string> getLicenses(const std::string& ownerRef) const;
        [[nodiscard]] size_t licenseCount(const std::string& ownerRef) const;

    private:
        void onLicenseAwarded(const leaderboard::LicenseAwardedEvent& e);

        core::EventBus& eventBus_;
        std::unordered_map<std::string, std::unordered_set<std::string>> ownerLicenses_;
    };

} // namespace kotr::licenses