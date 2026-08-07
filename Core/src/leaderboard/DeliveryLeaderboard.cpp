#include "kotr/leaderboard/DeliveryLeaderboard.hpp"
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
        double onTimeMult = e.onTime ? 1.0 : 0.7;
        double score = e.massTons * e.distanceKm * onTimeMult;

        bool isNamed = (e.kind == CarrierKind::Named || e.kind == CarrierKind::Player);
        carrierIsNamed_[e.carrierId] = isNamed;

        auto& entry = periodScores_[e.carrierId];
        entry.carrierId = e.carrierId;
        entry.isNamed = isNamed;
        entry.score += score;
        entry.deliveries++;
    }

    void DeliveryLeaderboard::onWeekElapsed(const GameWeekElapsed&) { evaluatePeriod(); }

    void DeliveryLeaderboard::evaluatePeriod() {
        if (periodScores_.empty()) {
            lastWeekWinnerId_ = "";
            periodCount_++;
            return;
        }

        std::vector<LeaderboardEntry> sorted;
        for (const auto& [id, entry] : periodScores_) sorted.push_back(entry);
        std::sort(sorted.begin(), sorted.end(), [](const LeaderboardEntry& a, const LeaderboardEntry& b) {
            return a.score > b.score;
            });

        std::string currentWinnerId = "";
        for (const auto& entry : sorted) {
            if (entry.isNamed && entry.score > 0.0) {
                currentWinnerId = entry.carrierId;
                break;
            }
        }

        if (!currentWinnerId.empty()) {
            if (currentWinnerId == lastWeekWinnerId_) carrierStreak_[currentWinnerId]++;
            else carrierStreak_[currentWinnerId] = 1;

            int streak = carrierStreak_[currentWinnerId];
            int licensesAwarded = (streak >= 3) ? 3 : streak;

            eventBus_.publish(LicenseAwardedEvent{
                currentWinnerId, carrierIsNamed_[currentWinnerId], licensesAwarded, sorted[0].score
                });
            lastWeekWinnerId_ = currentWinnerId;
        }
        else {
            lastWeekWinnerId_ = "";
        }

        periodScores_.clear();
        periodCount_++;
    }

    LeaderboardData DeliveryLeaderboard::getLeaderboard() const {
        LeaderboardData result;
        for (const auto& [id, entry] : periodScores_) result.entries.push_back(entry);
        std::sort(result.entries.begin(), result.entries.end(), [](const LeaderboardEntry& a, const LeaderboardEntry& b) {
            return a.score > b.score;
            });
        return result;
    }

    const LeaderboardEntry* DeliveryLeaderboard::getEntry(const std::string& carrierId) const {
        auto it = periodScores_.find(carrierId);
        return it != periodScores_.end() ? &it->second : nullptr;
    }

    std::vector<std::string> DeliveryLeaderboard::getTopCarriers(int count) const {
        auto lb = getLeaderboard();
        std::vector<std::string> result;
        for (int i = 0; i < count && i < static_cast<int>(lb.entries.size()); ++i) result.push_back(lb.entries[i].carrierId);
        return result;
    }
}