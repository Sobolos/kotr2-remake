#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "kotr/core/EventBus.hpp"
#include "kotr/core/TimeSystem.hpp"

using namespace kotr::core;

// === GameTime ===

TEST_CASE("GameTime: basic calculations") {
    GameTime t{0};
    CHECK(t.hourOfDay() == 0);
    CHECK(t.minuteOfHour() == 0);
    CHECK(t.day() == 0);

    SUBCASE("1 hour") {
        t.totalMinutes = 60;
        CHECK(t.hourOfDay() == 1);
        CHECK(t.minuteOfHour() == 0);
    }

    SUBCASE("1 day") {
        t.totalMinutes = 60 * 24;
        CHECK(t.day() == 1);
        CHECK(t.hourOfDay() == 0);
    }

    SUBCASE("night detection") {
        t.totalMinutes = 23 * 60; // 23:00
        CHECK(t.isNight());

        t.totalMinutes = 12 * 60; // 12:00
        CHECK_FALSE(t.isNight());

        t.totalMinutes = 5 * 60; // 05:00
        CHECK(t.isNight());
    }
}

// === EventBus ===

struct TestEvent {
    int value;
};

TEST_CASE("EventBus: publish and receive") {
    EventBus bus;
    int received = 0;

    bus.subscribe<TestEvent>([&](const TestEvent& e) {
        received += e.value;
    });

    bus.publish(TestEvent{42});
    CHECK(received == 42);

    bus.publish(TestEvent{8});
    CHECK(received == 50);
}

TEST_CASE("EventBus: multiple subscribers") {
    EventBus bus;
    int count = 0;

    bus.subscribe<TestEvent>([&](const TestEvent&) { count++; });
    bus.subscribe<TestEvent>([&](const TestEvent&) { count++; });

    bus.publish(TestEvent{1});
    CHECK(count == 2);
}

TEST_CASE("EventBus: unsubscribe") {
    EventBus bus;
    int count = 0;

    auto id = bus.subscribe<TestEvent>([&](const TestEvent&) { count++; });
    bus.publish(TestEvent{1});
    CHECK(count == 1);

    bus.unsubscribe<TestEvent>(id);
    bus.publish(TestEvent{1});
    CHECK(count == 1); // не изменилось
}

TEST_CASE("EventBus: no subscribers is safe") {
    EventBus bus;
    bus.publish(TestEvent{1}); // не должно падать
}

// === TimeSystem ===

TEST_CASE("TimeSystem: tick advances time") {
    EventBus bus;
    TimeSystem time(bus);

    CHECK(time.currentTime().totalMinutes == 0);

    // 1 реальная секунда × 4 = 4 игровых секунды < 1 минуты
    time.tick(1.0);
    CHECK(time.currentTime().totalMinutes == 0);

    // Ещё 14 секунд → итого 15 реальных × 4 = 60 игровых секунд = 1 минута
    time.tick(14.0);
    CHECK(time.currentTime().totalMinutes == 1);
}

TEST_CASE("TimeSystem: publishes hour events") {
    EventBus bus;
    TimeSystem time(bus);

    int hourCount = 0;
    int lastHour = -1;

    bus.subscribe<GameHourElapsed>([&](const GameHourElapsed& e) {
        hourCount++;
        lastHour = e.hour;
    });

    // 15 реальных минут × 4 = 60 игровых минут = 1 час
    time.tick(15.0 * 60.0); // 900 реальных секунд
    CHECK(hourCount == 1);
    CHECK(lastHour == 1);
}

TEST_CASE("TimeSystem: publishes day events") {
    EventBus bus;
    TimeSystem time(bus);

    int dayCount = 0;
    bus.subscribe<GameDayElapsed>([&](const GameDayElapsed& e) {
        dayCount++;
    });

    // 1 игровой день = 24 часа × 15 реальных минут = 360 реальных минут = 6 часов
    time.tick(360.0 * 60.0); // 21600 реальных секунд
    CHECK(dayCount == 1);
}

TEST_CASE("TimeSystem: pause stops time") {
    EventBus bus;
    TimeSystem time(bus);

    time.pause();
    time.tick(1000.0);
    CHECK(time.currentTime().totalMinutes == 0);

    time.resume();
    time.tick(15.0); // 1 минута
    CHECK(time.currentTime().totalMinutes == 1);
}

TEST_CASE("TimeSystem: custom time scale") {
    EventBus bus;
    TimeSystem time(bus);

    time.setTimeScale(8.0); // ускорение ×8

    // 7.5 реальных секунд × 8 = 60 игровых секунд = 1 минута
    time.tick(7.5);
    CHECK(time.currentTime().totalMinutes == 1);
}