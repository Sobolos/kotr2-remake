#include "kotr/leaderboard/DeliveryLeaderboard.hpp"
#include "kotr/contracts/ContractSystem.hpp"
#include <algorithm>

namespace kotr::leaderboard {

    using namespace kotr::core;
    using namespace kotr::contracts;

    DeliveryLeaderboard::DeliveryLeaderboard(EventBus& eventBus, TimeSystem& time)
        : eventBus_(eventBus), time_(time) {
        eventBus_.subscribe<ContractDeliveryArrivedEvent>(
            [this](const ContractDeliveryArrivedEvent& e) { onDelivery(e); });
        eventBus_.subscribe<GameWeekElapsed>(
            [this](const GameWeekElapsed& e) { onWeekElapsed(e); });
    }

    void DeliveryLeaderboard::onDelivery(const ContractDeliveryArrivedEvent& e) {
        // По Driver Design §4:
        // Score = mass * distance * category_mult * on_time * condition
        // Упрощённо: mass * distance * category_mult * on_time (condition = 1.0)

        LeaderboardCategory cat = goodToCategory(e.good); // нужно добавить поле good в событие
        double catMult = categoryMultiplier(cat);
        double onTimeMult = e.onTime ? 1.0 : 0.7;
        double score = e.massTons * e.distanceKm * catMult * onTimeMult;

        bool isNamed = (e.kind == CarrierKind::Named || e.kind == CarrierKind::Player);
        carrierIsNamed_[e.carrierId] = isNamed;

        // Обновляем score для категории
        auto& catScores = periodScores_[cat];
        auto& entry = catScores[e.carrierId];
        entry.carrierId = e.carrierId;
        entry.isNamed = isNamed;
        entry.score += score;
        entry.deliveries++;

        // Также обновляем Overall
        auto& overallScores = periodScores_[LeaderboardCategory::Overall];
        auto& overallEntry = overallScores[e.carrierId];
        overallEntry.carrierId = e.carrierId;
        overallEntry.isNamed = isNamed;
        overallEntry.score += score;
        overallEntry.deliveries++;
    }

    void DeliveryLeaderboard::onWeekElapsed(const GameWeekElapsed&) {
        evaluatePeriod();
    }

    void DeliveryLeaderboard::evaluatePeriod() {
        // Определяем победителей в каждой категории
        for (const auto& [cat, entries] : periodScores_) {
            if (entries.empty()) continue;

            // Сортируем по score (desc)
            std::vector<LeaderboardEntry> sorted;
            for (const auto& [id, entry] : entries) {
                sorted.push_back(entry);
            }
            std::sort(sorted.begin(), sorted.end(),
                [](const LeaderboardEntry& a, const LeaderboardEntry& b) {
                    return a.score > b.score;
                });

            // Победитель = первый eligible (player/named)
            for (const auto& entry : sorted) {
                if (entry.isNamed && entry.score > 0.0) {
                    std::string licenseKey = categoryToLicenseKey(cat);
                    eventBus_.publish(LicenseAwardedEvent{
                        entry.carrierId, entry.isNamed, licenseKey, entry.score });
                    break;
                }
            }
        }

        // Очищаем период
        periodScores_.clear();
        periodCount_++;
    }

    LeaderboardCategory DeliveryLeaderboard::goodToCategory(const GoodId& goodId) {
        if (goodId == "good_food") return LeaderboardCategory::Food;
        if (goodId == "good_timber") return LeaderboardCategory::Timber;
        if (goodId == "good_coal") return LeaderboardCategory::Coal;
        if (goodId == "good_fuel") return LeaderboardCategory::Fuel;
        if (goodId == "good_parts") return LeaderboardCategory::Consumer;
        if (goodId == "good_metal") return LeaderboardCategory::Metal;
        if (goodId == "good_ore") return LeaderboardCategory::Ore;
        if (goodId == "good_gems") return LeaderboardCategory::Valuables;
        if (goodId == "good_consumer") return LeaderboardCategory::Consumer;
        return LeaderboardCategory::Overall;
    }

    double DeliveryLeaderboard::categoryMultiplier(LeaderboardCategory cat) {
        switch (cat) {
        case LeaderboardCategory::Food:      return 1.0;
        case LeaderboardCategory::Timber:    return 0.9;
        case LeaderboardCategory::Coal:      return 0.9;
        case LeaderboardCategory::Fuel:      return 1.1;
        case LeaderboardCategory::Consumer:  return 1.0;
        case LeaderboardCategory::Metal:     return 1.1;
        case LeaderboardCategory::Ore:       return 1.0;
        case LeaderboardCategory::Valuables: return 1.5;
        case LeaderboardCategory::Illegal:   return 1.3;
        case LeaderboardCategory::Overall:   return 1.0;
        }
        return 1.0;
    }

    std::string DeliveryLeaderboard::categoryToLicenseKey(LeaderboardCategory cat) {
        switch (cat) {
        case LeaderboardCategory::Food:      return "License.Food";
        case LeaderboardCategory::Timber:    return "License.Timber";
        case LeaderboardCategory::Coal:      return "License.Coal";
        case LeaderboardCategory::Fuel:      return "License.Fuel";
        case LeaderboardCategory::Consumer:  return "License.Consumer";
        case LeaderboardCategory::Metal:     return "License.Metal";
        case LeaderboardCategory::Ore:       return "License.Ore";
        case LeaderboardCategory::Valuables: return "License.Valuables";
        case LeaderboardCategory::Illegal:   return "License.Illegal";
        case LeaderboardCategory::Overall:   return "License.TopHauler";
        }
        return "License.Overall";
    }

    CategoryLeaderboard DeliveryLeaderboard::getLeaderboard(LeaderboardCategory cat) const {
        CategoryLeaderboard result;
        result.category = cat;

        auto it = periodScores_.find(cat);
        if (it != periodScores_.end()) {
            for (const auto& [id, entry] : it->second) {
                result.entries.push_back(entry);
            }
            std::sort(result.entries.begin(), result.entries.end(),
                [](const LeaderboardEntry& a, const LeaderboardEntry& b) {
                    return a.score > b.score;
                });
        }

        return result;
    }

    const LeaderboardEntry* DeliveryLeaderboard::getEntry(LeaderboardCategory cat,
        const std::string& carrierId) const {
        auto catIt = periodScores_.find(cat);
        if (catIt == periodScores_.end()) return nullptr;

        auto entryIt = catIt->second.find(carrierId);
        return entryIt != catIt->second.end() ? &entryIt->second : nullptr;
    }

    std::vector<std::string> DeliveryLeaderboard::getTopCarriers(LeaderboardCategory cat, int count) const {
        auto lb = getLeaderboard(cat);
        std::vector<std::string> result;
        for (int i = 0; i < count && i < static_cast<int>(lb.entries.size()); ++i) {
            result.push_back(lb.entries[i].carrierId);
        }
        return result;
    }

} // namespace kotr::leaderboard