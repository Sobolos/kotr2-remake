#include "kotr/core/TimeSystem.hpp"
#include "kotr/core/EventBus.hpp"

namespace kotr::core {

TimeSystem::TimeSystem(EventBus& eventBus)
    : eventBus_(eventBus) {}

void TimeSystem::tick(double deltaRealSeconds) {
    if (paused_ || deltaRealSeconds <= 0.0) return;

    accumulatedRealSeconds_ += deltaRealSeconds;

    // 1 реальная секунда × timeScale = timeScale игровых секунд
    // gameMinutes = gameSeconds / 60
    double gameSeconds = accumulatedRealSeconds_ * timeScale_;
    auto gameMinutes = static_cast<int64_t>(gameSeconds / 60.0);

    if (gameMinutes <= 0) return;

    // Вычитаем обработанные минуты из аккумулятора
    accumulatedRealSeconds_ -= (static_cast<double>(gameMinutes) * 60.0) / timeScale_;

    int64_t previousMinutes = currentTime_.totalMinutes;
    currentTime_ += gameMinutes;

    publishTimeEvents(previousMinutes, currentTime_.totalMinutes);
}

void TimeSystem::publishTimeEvents(int64_t previousMinutes, int64_t newMinutes) {
    // --- Часы ---
    int64_t prevHour = previousMinutes / 60;
    int64_t newHour  = newMinutes / 60;
    for (int64_t h = prevHour + 1; h <= newHour; ++h) {
        GameTime t{h * 60};
        eventBus_.publish(GameHourElapsed{t, t.hourOfDay()});
    }

    // --- Дни ---
    int64_t prevDay = previousMinutes / (60 * 24);
    int64_t newDay  = newMinutes / (60 * 24);
    for (int64_t d = prevDay + 1; d <= newDay; ++d) {
        GameTime t{d * 60 * 24};
        eventBus_.publish(GameDayElapsed{t, t.day()});
    }

    // --- Недели ---
    int64_t prevWeek = previousMinutes / (60 * 24 * 7);
    int64_t newWeek  = newMinutes / (60 * 24 * 7);
    for (int64_t w = prevWeek + 1; w <= newWeek; ++w) {
        GameTime t{w * 60 * 24 * 7};
        eventBus_.publish(GameWeekElapsed{t, t.week()});
    }
}

} // namespace kotr::core