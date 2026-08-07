#pragma once
#include <cstdint>

namespace kotr::core {

// === Игровое время ===

/// Время в игровых минутах. 0 = начало игры (День 1, 00:00).
struct GameTime {
    int64_t totalMinutes = 0;

    [[nodiscard]] int minuteOfHour() const { return static_cast<int>(totalMinutes % 60); }
    [[nodiscard]] int hourOfDay()    const { return static_cast<int>((totalMinutes / 60) % 24); }
    [[nodiscard]] int dayOfWeek()    const { return static_cast<int>((totalMinutes / (60 * 24)) % 7); }
    [[nodiscard]] int day()          const { return static_cast<int>(totalMinutes / (60 * 24)); }
    [[nodiscard]] int week()         const { return static_cast<int>(totalMinutes / (60 * 24 * 7)); }

    /// Ночь: 22:00 – 06:00 (для риск-системы)
    [[nodiscard]] bool isNight() const {
        int h = hourOfDay();
        return h >= 22 || h < 6;
    }

    GameTime& operator+=(int64_t minutes) { totalMinutes += minutes; return *this; }
    [[nodiscard]] bool operator>=(const GameTime& o) const { return totalMinutes >= o.totalMinutes; }
    [[nodiscard]] bool operator<(const GameTime& o)  const { return totalMinutes < o.totalMinutes; }
};

// === События времени (публикуются TimeSystem) ===

struct GameHourElapsed {
    GameTime time;
    int hour;       // 0–23
};

struct GameDayElapsed {
    GameTime time;
    int day;        // номер дня (0-based)
};

struct GameWeekElapsed {
    GameTime time;
    int week;       // номер недели (0-based)
};

// === TimeSystem ===

class EventBus;

/// Система игрового времени.
/// Масштаб 1:4 — 1 реальная секунда = 4 игровых секунды.
/// Публикует GameHourElapsed, GameDayElapsed, GameWeekElapsed.
class TimeSystem {
public:
    static constexpr double DEFAULT_TIME_SCALE = 4.0;

    explicit TimeSystem(EventBus& eventBus);

    /// Обновить время. deltaRealSeconds — прошедшее реальное время.
    void tick(double deltaRealSeconds);

    // --- Геттеры ---
    [[nodiscard]] GameTime currentTime() const { return currentTime_; }
    [[nodiscard]] int currentDay()  const { return currentTime_.day(); }
    [[nodiscard]] int currentHour() const { return currentTime_.hourOfDay(); }
    [[nodiscard]] bool isNight()    const { return currentTime_.isNight(); }

    // --- Управление скоростью ---
    void setTimeScale(double scale) { timeScale_ = scale; }
    [[nodiscard]] double timeScale() const { return timeScale_; }

    void pause()  { paused_ = true; }
    void resume() { paused_ = false; }
    [[nodiscard]] bool isPaused() const { return paused_; }

private:
    void publishTimeEvents(int64_t previousMinutes, int64_t newMinutes);

    EventBus& eventBus_;
    GameTime currentTime_;
    double timeScale_ = DEFAULT_TIME_SCALE;
    bool paused_ = false;
    double accumulatedRealSeconds_ = 0.0;
};

} // namespace kotr::core