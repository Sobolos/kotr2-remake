#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "kotr/core/EventBus.hpp"
#include "kotr/core/TimeSystem.hpp"
#include "kotr/drivers/DriverSystem.hpp"
#include "kotr/finance/FinanceSystem.hpp"
#include "kotr/market/MarketSystem.hpp"

using namespace kotr::core;
using namespace kotr::drivers;
using namespace kotr::finance;
using namespace kotr::market;

// === Вспомогательные функции ===

static DriverPersonaData makePersona(const std::string& id, double capacityKg = 10000,
    Money buyout = 25000) {
    DriverPersonaData p;
    p.id = id;
    p.nameKey = "loc." + id + ".name";
    p.homeCity = "city_yuzhny";
    p.vehicleCapacityKg = capacityKg;
    p.vehicleBuyoutPrice = buyout;
    p.signingBonus = 500;
    p.salaryExpectation = 200;
    p.paymentPercent = 10;
    return p;
}

static void registerTestDrivers(DriverSystem& ds, int count) {
    for (int i = 0; i < count; ++i) {
        ds.registerDriver(makePersona("drv_" + std::to_string(i)));
    }
}

// === Тесты: Рынок труда ===

TEST_CASE("DriverSystem: labor market visibility cap") {
    EventBus bus;
    TimeSystem time(bus);
    DriverSystem ds(bus, time);

    SUBCASE("Максимум 10 видимых кандидатов") {
        registerTestDrivers(ds, 12);
        CHECK(ds.getVisibleCandidates().size() == 10);

        // Двое ушли в независимые
        const auto* st = ds.getDriverState("drv_10");
        CHECK(st->status == DriverEmploymentStatus::FreeIndependent);
    }

    SUBCASE("Видимые кандидаты сортированы") {
        registerTestDrivers(ds, 5);
        auto visible = ds.getVisibleCandidates();
        CHECK(visible.size() == 5);
        CHECK(visible.front() == "drv_0");
    }
}

TEST_CASE("DriverSystem: market rotation") {
    EventBus bus;
    TimeSystem time(bus);
    DriverSystem ds(bus, time);
    registerTestDrivers(ds, 5);

    bool rotated = false;
    bus.subscribe<LaborMarketRotatedEvent>([&](const LaborMarketRotatedEvent&) {
        rotated = true;
        });

    ds.rotateMarket();
    CHECK(rotated);
    CHECK(ds.getVisibleCandidates().size() == 5); // слот освободился и занялся
}

// === Тесты: Найм ===

TEST_CASE("DriverSystem: hiring requires licenses") {
    EventBus bus;
    TimeSystem time(bus);
    DriverSystem ds(bus, time);

    DriverPersonaData p = makePersona("drv_fuel_pro");
    p.requiredLicenses = { "License.Fuel" };
    ds.registerDriver(p);

    SUBCASE("Без лицензии найм невозможен") {
        CHECK_FALSE(ds.hasRequiredLicenses("drv_fuel_pro"));
        CHECK_FALSE(ds.hireDriver("drv_fuel_pro"));
    }

    SUBCASE("С лицензией найм возможен") {
        ds.addPlayerLicense("License.Fuel");
        CHECK(ds.hasRequiredLicenses("drv_fuel_pro"));
        CHECK(ds.hireDriver("drv_fuel_pro"));

        const auto* st = ds.getDriverState("drv_fuel_pro");
        CHECK(st->status == DriverEmploymentStatus::HiredByPlayer);
        CHECK(st->employerRef == "player");
    }
}

TEST_CASE("DriverSystem: hire cost breakdown") {
    EventBus bus;
    TimeSystem time(bus);
    DriverSystem ds(bus, time);
    ds.registerDriver(makePersona("drv_ivan", 10000, 25000));

    auto cost = ds.calcHireCost("drv_ivan");
    CHECK(cost.signingBonus == 500);
    CHECK(cost.vehicleBuyout == 25000);
    CHECK(cost.total == 25500);
}

TEST_CASE("DriverSystem: hired capacity and market integration") {
    EventBus bus;
    TimeSystem time(bus);
    DriverSystem ds(bus, time);
    MarketSystem market(bus);

    // Интеграция: найм увеличивает ёмкость игрока (event-driven по TAD)
    bus.subscribe<DriverHiredEvent>([&](const DriverHiredEvent& e) {
        market.adjustPlayerCapacity(e.capacityKg);
        });

    market.setCompetitorCapacity("others", 1260000);
    market.setPlayerCapacity(5000); // ЗиЛ игрока

    ds.registerDriver(makePersona("drv_ivan", 10000, 25000));
    ds.hireDriver("drv_ivan");

    SUBCASE("Ёмкость нанятых считается") {
        CHECK(ds.getHiredCapacityKg() == doctest::Approx(10000.0));
    }

    SUBCASE("Доля рынка выросла после найма") {
        // Player: 5000 + 10000 = 15000; Total = 15000 + 1260000
        CHECK(market.playerCapacity() == doctest::Approx(15000.0));
        CHECK(market.getMarketShare() > 1.0);
    }
}

TEST_CASE("DriverSystem: firing returns to market") {
    EventBus bus;
    TimeSystem time(bus);
    DriverSystem ds(bus, time);
    ds.registerDriver(makePersona("drv_ivan"));
    ds.hireDriver("drv_ivan");

    CHECK(ds.fireDriver("drv_ivan"));

    const auto* st = ds.getDriverState("drv_ivan");
    CHECK(st->status == DriverEmploymentStatus::FreeParked);
    CHECK(st->employerRef.empty());

    auto visible = ds.getVisibleCandidates();
    CHECK(std::find(visible.begin(), visible.end(), std::string("drv_ivan")) != visible.end());
}

// === Тесты: Зарплаты ===

TEST_CASE("DriverSystem: salary by payment policy") {
    EventBus bus;
    TimeSystem time(bus);
    DriverSystem ds(bus, time);
    ds.registerDriver(makePersona("drv_ivan")); // expectation=200, percent=10
    ds.hireDriver("drv_ivan");

    Money revenue = 5000;

    SUBCASE("Fixed: фикс в день") {
        CHECK(ds.calcDailySalary("drv_ivan", revenue) == 200);
    }

    SUBCASE("Percent: 10% от выручки") {
        ds.getDriverState("drv_ivan"); // доступ к состоянию
        // Меняем политику через прямой доступ к состоянию (для теста)
        const_cast<DriverState*>(ds.getDriverState("drv_ivan"))->paymentPolicy = PaymentPolicy::Percent;
        CHECK(ds.calcDailySalary("drv_ivan", revenue) == 500);
    }

    SUBCASE("FixedPlusBonus: 0.6*оклад + 0.05*выручка") {
        const_cast<DriverState*>(ds.getDriverState("drv_ivan"))->paymentPolicy = PaymentPolicy::FixedPlusBonus;
        CHECK(ds.calcDailySalary("drv_ivan", revenue) == 370); // 120 + 250
    }

    SUBCASE("BonusOnly: 15% от выручки") {
        const_cast<DriverState*>(ds.getDriverState("drv_ivan"))->paymentPolicy = PaymentPolicy::BonusOnly;
        CHECK(ds.calcDailySalary("drv_ivan", revenue) == 750);
    }
}

TEST_CASE("DriverSystem: daily update pays salaries and adds fatigue") {
    EventBus bus;
    TimeSystem time(bus);
    DriverSystem ds(bus, time);
    FinanceSystem finance(bus);
    finance.setInitialBalance(10000);

    ds.registerDriver(makePersona("drv_ivan"));
    ds.hireDriver("drv_ivan");

    // Интеграция: зарплаты — расход игрока
    bus.subscribe<DriverSalaryDueEvent>([&](const DriverSalaryDueEvent& e) {
        finance.addExpense(e.amount, TransactionCategory::DriverSalary, "Salary " + e.driverId);
        });

    // Симулируем 1 игровой день
    ds.updateDay(GameTime{ 60 * 24 });

    const auto* st = ds.getDriverState("drv_ivan");
    CHECK(st->fatigue == 10);
    CHECK(finance.balance() == 10000 - 200); // Fixed зарплата
}

// === Тесты: Риск предательства ===

TEST_CASE("DriverSystem: betrayal risk formula") {
    EventBus bus;
    TimeSystem time(bus);
    DriverSystem ds(bus, time);

    // Лояльный водитель
    DriverPersonaData loyal = makePersona("drv_loyal");
    ds.registerDriver(loyal);
    ds.hireDriver("drv_loyal");
    const_cast<DriverState*>(ds.getDriverState("drv_loyal"))->loyalty = 90;
    const_cast<DriverState*>(ds.getDriverState("drv_loyal"))->morale = 80;
    const_cast<DriverState*>(ds.getDriverState("drv_loyal"))->fatigue = 10;

    // Предатель с низкой лояльностью
    DriverPersonaData traitor = makePersona("drv_traitor");
    traitor.hiddenTags = { "Driver.Hidden.Traitor" };
    ds.registerDriver(traitor);
    ds.hireDriver("drv_traitor");
    const_cast<DriverState*>(ds.getDriverState("drv_traitor"))->loyalty = 20;
    const_cast<DriverState*>(ds.getDriverState("drv_traitor"))->morale = 30;
    const_cast<DriverState*>(ds.getDriverState("drv_traitor"))->fatigue = 60;
    const_cast<DriverState*>(ds.getDriverState("drv_traitor"))->paymentPolicy = PaymentPolicy::BonusOnly;

    double riskLoyal = ds.calcBetrayalRisk("drv_loyal");
    double riskTraitor = ds.calcBetrayalRisk("drv_traitor");

    CHECK(riskLoyal >= 0.0);
    CHECK(riskLoyal <= 1.0);
    CHECK(riskTraitor > riskLoyal);
    CHECK(riskTraitor > 0.5); // Предатель в плохом состоянии очень опасен
}

// === Тесты: Уничтожение машины и Ersatz ===

TEST_CASE("DriverSystem: vehicle destruction and ersatz return") {
    EventBus bus;
    TimeSystem time(bus);
    DriverSystem ds(bus, time);
    ds.registerDriver(makePersona("drv_ivan", 10000, 25000));
    ds.hireDriver("drv_ivan");

    // День 1: машина уничтожена
    ds.driverVehicleDestroyed("drv_ivan", GameTime{ 1440 });

    const auto* st = ds.getDriverState("drv_ivan");
    CHECK(st->status == DriverEmploymentStatus::OutOfAction);
    CHECK(st->employerRef.empty());          // соглашение расторгнуто без штрафа
    CHECK(st->currentVehicleCapacityKg == 0.0);
    CHECK(st->outOfActionUntil == 1440 + 1440); // +1 день

    // День 2: водитель возвращается с Ersatz-тягачом
    bool returned = false;
    double ersatzKg = 0.0;
    bus.subscribe<DriverReturnedEvent>([&](const DriverReturnedEvent& e) {
        returned = true;
        ersatzKg = e.ersatzCapacityKg;
        });

    ds.updateDay(GameTime{ 2880 });

    CHECK(returned);
    CHECK(ersatzKg == doctest::Approx(5000.0)); // 50% от оригинала

    st = ds.getDriverState("drv_ivan");
    CHECK(st->status == DriverEmploymentStatus::FreeParked); // вернулся свободным
    CHECK(st->currentVehicleCapacityKg == doctest::Approx(5000.0));
}

// === Тесты: Отдых ===

TEST_CASE("DriverSystem: rest reduces fatigue") {
    EventBus bus;
    TimeSystem time(bus);
    DriverSystem ds(bus, time);
    ds.registerDriver(makePersona("drv_ivan"));
    ds.hireDriver("drv_ivan");

    const_cast<DriverState*>(ds.getDriverState("drv_ivan"))->fatigue = 80;
    ds.sendToRest("drv_ivan");

    ds.updateDay(GameTime{ 1440 });

    const auto* st = ds.getDriverState("drv_ivan");
    CHECK(st->fatigue == 50); // 80 - 30
    CHECK(st->status == DriverEmploymentStatus::Resting); // ещё отдыхает (>20)
}