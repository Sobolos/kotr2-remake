#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "kotr/core/EventBus.hpp"
#include "kotr/core/TimeSystem.hpp"
#include "kotr/economy/EconomySystem.hpp"
#include "kotr/contracts/ContractSystem.hpp"
#include "kotr/drivers/DriverSystem.hpp"
#include "kotr/drivers/DriverAgentSystem.hpp"

using namespace kotr::core;
using namespace kotr::data;
using namespace kotr::economy;
using namespace kotr::contracts;
using namespace kotr::drivers;

// Мини-мир: a — источник, b — дефицит, c — нейтральный сосед
static void buildWorld(EconomySystem& eco) {
    GoodData food;
    food.id = "good_food";
    food.basePricePerTon = 650;
    food.baseTariffPerTonKm = 4;
    food.dangerModifier = 0.1;
    food.legal = true;
    eco.registerGood(food);

    CityData a; a.id = "city_a"; a.coordX = 0; a.coordY = 0; a.baseDanger = 0.2;
    CityData b; b.id = "city_b"; b.coordX = 0; b.coordY = 3; b.baseDanger = 0.3;
    CityData c; c.id = "city_c"; c.coordX = 0; c.coordY = 5; c.baseDanger = 0.2;
    eco.registerCity(a); eco.registerCity(b); eco.registerCity(c);

    CityGoodState ga; ga.goodId = "good_food"; ga.supply = 100; ga.demand = 5;
    eco.initCityGood("city_a", ga);
    CityGoodState gb; gb.goodId = "good_food"; gb.supply = 10; gb.demand = 40;
    eco.initCityGood("city_b", gb);
    CityGoodState gc; gc.goodId = "good_food"; gc.supply = 20; gc.demand = 5;
    eco.initCityGood("city_c", gc);
}

TEST_CASE("DriverAgentSystem: registration and competitor capacity") {
    EventBus bus;
    TimeSystem time(bus);
    EconomySystem eco(bus, EconomyMode::Living);
    buildWorld(eco);
    ContractSystem contracts(bus, eco, time);
    DriverSystem drivers(bus, time);
    DriverAgentSystem agents(bus, eco, contracts, drivers, time);

    agents.addMassAI("ai_1", 7500, 1.0, "city_a");
    agents.addNamedAgent("drv_a", 10000, 0.0, "city_a");
    agents.addNamedAgent("drv_b", 10000, 0.5, "city_a");

    CHECK(agents.agentCount() == 3);
    CHECK(agents.getCompetitorCapacityKg() == doctest::Approx(27500.0));

    SUBCASE("Найм игроком уводит ёмкость из конкурентов") {
        DriverPersonaData p; p.id = "drv_a"; p.vehicleCapacityKg = 10000;
        drivers.registerDriver(p);
        CHECK(drivers.hireDriver("drv_a", "player"));
        CHECK(agents.getCompetitorCapacityKg() == doctest::Approx(17500.0));
    }

    SUBCASE("Именной нанимает именного: ёмкость остаётся у конкурентов") {
        DriverPersonaData pa; pa.id = "drv_a"; pa.vehicleCapacityKg = 10000;
        DriverPersonaData pb; pb.id = "drv_b"; pb.vehicleCapacityKg = 10000;
        drivers.registerDriver(pa); drivers.registerDriver(pb);
        CHECK(drivers.hireDriver("drv_b", "drv_a"));
        CHECK(agents.getCompetitorCapacityKg() == doctest::Approx(27500.0));
    }
}

TEST_CASE("DriverAgentSystem: takes order at own base and relocates") {
    EventBus bus;
    TimeSystem time(bus);
    EconomySystem eco(bus, EconomyMode::Living);
    buildWorld(eco);
    ContractSystem contracts(bus, eco, time);
    DriverSystem drivers(bus, time);
    DriverAgentSystem agents(bus, eco, contracts, drivers, time);
    agents.setBaseTakeRate(1.0); // детерминированный захват при interest >= MIN

    agents.addMassAI("ai_1", 15000, 1.0, "city_a");

    contracts.generateContracts(GameTime{ 0 });
    agents.updateHour(GameTime{ 0 });

    const auto* a = agents.getAgent("ai_1");
    REQUIRE(a != nullptr);
    CHECK(a->busy);

    contracts.resolveDeliveries(GameTime{ 60 });

    a = agents.getAgent("ai_1");
    CHECK_FALSE(a->busy);
    CHECK(a->currentBase == "city_b"); // переехал в базу назначения
    CHECK(a->failedTakeAttempts == 0);
}

TEST_CASE("DriverAgentSystem: neighbor base adds reposition delay") {
    EventBus bus;
    TimeSystem time(bus);
    EconomySystem eco(bus, EconomyMode::Living);
    buildWorld(eco);
    ContractSystem contracts(bus, eco, time);
    DriverSystem drivers(bus, time);
    DriverAgentSystem agents(bus, eco, contracts, drivers, time);
    agents.setBaseTakeRate(1.0);

    agents.addMassAI("ai_local", 15000, 1.0, "city_a");   // на базе истока
    agents.addMassAI("ai_far", 15000, 1.0, "city_c");     // у соседа

    contracts.generateContracts(GameTime{ 0 });
    agents.updateHour(GameTime{ 0 });

    // Локальный доехал к t=5 (4 мин), соседний ещё в пути (перегон 7 + 4 = 11)
    int deliveredAt5 = 0;
    contracts.resolveDeliveries(GameTime{ 5 });
    CHECK(agents.getAgent("ai_local")->busy == false);
    CHECK(agents.getAgent("ai_far")->busy == true);

    contracts.resolveDeliveries(GameTime{ 15 });
    CHECK(agents.getAgent("ai_far")->busy == false);
}

TEST_CASE("DriverAgentSystem: mercy rule on third visit") {
    EventBus bus;
    TimeSystem time(bus);
    EconomySystem eco(bus, EconomyMode::Living);

    // Слабый дефицит: interest = (57/52 - 0.8)*0.5 ≈ 0.148 < MIN_TAKE_INTEREST
    GoodData food; food.id = "good_food"; food.baseTariffPerTonKm = 4; food.legal = true;
    eco.registerGood(food);
    CityData a; a.id = "city_a"; a.coordX = 0; a.coordY = 0;
    CityData b; b.id = "city_b"; b.coordX = 0; b.coordY = 3;
    eco.registerCity(a); eco.registerCity(b);
    CityGoodState ga; ga.goodId = "good_food"; ga.supply = 52; ga.demand = 2;
    eco.initCityGood("city_a", ga);
    CityGoodState gb; gb.goodId = "good_food"; gb.supply = 52; gb.demand = 57;
    eco.initCityGood("city_b", gb);

    ContractSystem contracts(bus, eco, time);
    DriverSystem drivers(bus, time);
    DriverAgentSystem agents(bus, eco, contracts, drivers, time);
    agents.setBaseTakeRate(1.0);

    agents.addMassAI("ai_1", 15000, 1.0, "city_a");
    contracts.generateContracts(GameTime{ 0 });
    REQUIRE(contracts.getAvailableContracts().size() == 1);

    int joined = 0;
    bus.subscribe<ContractJoinedEvent>([&](const ContractJoinedEvent&) { joined++; });

    agents.updateHour(GameTime{ 0 });    // визит 1: interest ниже порога — не берёт
    agents.updateHour(GameTime{ 60 });   // визит 2: всё ещё не берёт
    CHECK(joined == 0);
    CHECK(agents.getAgent("ai_1")->failedTakeAttempts == 2);

    agents.updateHour(GameTime{ 120 });  // визит 3: гарантированно берёт
    CHECK(joined == 1);
    CHECK(agents.getAgent("ai_1")->failedTakeAttempts == 0);
}

TEST_CASE("DriverAgentSystem: named employer earns minus salary") {
    EventBus bus;
    TimeSystem time(bus);
    EconomySystem eco(bus, EconomyMode::Living);
    buildWorld(eco);
    ContractSystem contracts(bus, eco, time);
    DriverSystem drivers(bus, time);
    DriverAgentSystem agents(bus, eco, contracts, drivers, time);
    agents.setBaseTakeRate(1.0);

    DriverPersonaData pa; pa.id = "drv_a"; pa.vehicleCapacityKg = 10000;
    DriverPersonaData pb; pb.id = "drv_b"; pb.vehicleCapacityKg = 10000;
    drivers.registerDriver(pa); drivers.registerDriver(pb);

    agents.addNamedAgent("drv_a", 10000, 0.5, "city_a");
    agents.addNamedAgent("drv_b", 10000, 1.0, "city_a");

    const_cast<DriverAgent*>(agents.getAgent("drv_a"))->money = 30000;

    // Именной A нанимает именного B: A платит выкуп + подъёмные (25500)
    CHECK(drivers.hireDriver("drv_b", "drv_a"));
    CHECK(agents.getAgent("drv_a")->money == 30000 - 25500);

    contracts.generateContracts(GameTime{ 0 });
    agents.updateHour(GameTime{ 0 }); // B берёт контракт (работает на A)

    Money payout = 0;
    bus.subscribe<ContractDeliveryArrivedEvent>([&](const ContractDeliveryArrivedEvent& e) {
        if (e.carrierId == "drv_b") payout = e.payout;
        });

    contracts.resolveDeliveries(GameTime{ 60 });
    REQUIRE(payout > 0);
    CHECK(agents.getAgent("drv_a")->money == 30000 - 25500 + payout);

    // Конец дня: A платит B зарплату (Fixed, expectation 150)
    drivers.updateDay(GameTime{ 1440 });
    CHECK(agents.getAgent("drv_a")->money == 30000 - 25500 + payout - 150);
}

TEST_CASE("DriverAgentSystem: named employer keeps working and earns from hires") {
    EventBus bus;
    TimeSystem time(bus);
    EconomySystem eco(bus, EconomyMode::Living);
    buildWorld(eco);
    ContractSystem contracts(bus, eco, time);
    DriverSystem drivers(bus, time);
    DriverAgentSystem agents(bus, eco, contracts, drivers, time);
    agents.setBaseTakeRate(1.0);

    DriverPersonaData pa; pa.id = "drv_a"; pa.vehicleCapacityKg = 10000;
    DriverPersonaData pb; pb.id = "drv_b"; pb.vehicleCapacityKg = 10000;
    drivers.registerDriver(pa); drivers.registerDriver(pb);

    // Работодатель A продолжает брать заказы сам (агрессия 1.0)
    agents.addNamedAgent("drv_a", 10000, 1.0, "city_a");
    agents.addNamedAgent("drv_b", 10000, 1.0, "city_a");

    const_cast<DriverAgent*>(agents.getAgent("drv_a"))->money = 30000;

    // A нанимает B: выкуп + подъёмные
    CHECK(drivers.hireDriver("drv_b", "drv_a"));

    contracts.generateContracts(GameTime{ 0 });
    agents.updateHour(GameTime{ 0 }); // A и B берут контракты

    Money totalPayout = 0;
    int deliveries = 0;
    bus.subscribe<ContractDeliveryArrivedEvent>([&](const ContractDeliveryArrivedEvent& e) {
        if (e.employerId == "drv_a") { totalPayout += e.payout; deliveries++; }
        });

    contracts.resolveDeliveries(GameTime{ 60 });
    REQUIRE(deliveries == 2); // A довёз свой рейс + B довёз свой

    // Конец дня: A платит зарплату B
    drivers.updateDay(GameTime{ 1440 });

    // Баланс A = старт − выкуп + (свой рейс + рейс B) − зарплата B
    CHECK(agents.getAgent("drv_a")->money == 30000 - 25500 + totalPayout - 150);
}

TEST_CASE("DriverAgentSystem: hired-by-player revenue routed to player") {
    EventBus bus;
    TimeSystem time(bus);
    EconomySystem eco(bus, EconomyMode::Living);
    buildWorld(eco);
    ContractSystem contracts(bus, eco, time);
    DriverSystem drivers(bus, time);
    DriverAgentSystem agents(bus, eco, contracts, drivers, time);
    agents.setBaseTakeRate(1.0);

    DriverPersonaData pb; pb.id = "drv_b"; pb.vehicleCapacityKg = 10000;
    drivers.registerDriver(pb);
    agents.addNamedAgent("drv_b", 10000, 1.0, "city_a");

    CHECK(drivers.hireDriver("drv_b", "player"));

    contracts.generateContracts(GameTime{ 0 });
    agents.updateHour(GameTime{ 0 });

    std::string employer;
    bus.subscribe<ContractDeliveryArrivedEvent>([&](const ContractDeliveryArrivedEvent& e) {
        if (e.carrierId == "drv_b") employer = e.employerId;
        });

    contracts.resolveDeliveries(GameTime{ 60 });
    CHECK(employer == "player"); // выручка маршрутизируется игроку
}