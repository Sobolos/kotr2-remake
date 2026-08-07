#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "kotr/core/EventBus.hpp"
#include "kotr/core/TimeSystem.hpp"
#include "kotr/economy/EconomySystem.hpp"
#include "kotr/contracts/ContractSystem.hpp"

using namespace kotr::core;
using namespace kotr::data;
using namespace kotr::economy;
using namespace kotr::contracts;

// Мини-мир: city_a — источник, city_b — дефицит
static void buildMiniWorld(EconomySystem& eco) {
    GoodData food;
    food.id = "good_food";
    food.category = GoodCategory::Consumer;
    food.basePricePerTon = 650;
    food.baseTariffPerTonKm = 4;
    food.dangerModifier = 0.1;
    food.legal = true;
    eco.registerGood(food);

    CityData a;
    a.id = "city_a"; a.coordX = 0; a.coordY = 0; a.baseDanger = 0.2;
    eco.registerCity(a);

    CityData b;
    b.id = "city_b"; b.coordX = 0; b.coordY = 3; b.baseDanger = 0.3;
    eco.registerCity(b);

    CityGoodState ga;
    ga.goodId = "good_food"; ga.supply = 100; ga.demand = 5;
    ga.productionPerDay = 50; ga.consumptionPerDay = 5;
    eco.initCityGood("city_a", ga);

    CityGoodState gb;
    gb.goodId = "good_food"; gb.supply = 10; gb.demand = 40;
    gb.productionPerDay = 0; gb.consumptionPerDay = 40;
    eco.initCityGood("city_b", gb);
}

static CarrierRef makeCarrier(const std::string& id, CarrierKind kind,
    double capacityKg, double speed = 50.0) {
    CarrierRef ref;
    ref.id = id;
    ref.kind = kind;
    ref.employerId = id;
    ref.capacityKg = capacityKg;
    ref.speedKmh = speed;
    return ref;
}

// === Генерация: контракт = пул спроса ===

TEST_CASE("ContractSystem: contract is a demand pool") {
    EventBus bus;
    TimeSystem time(bus);
    EconomySystem eco(bus, EconomyMode::Living);
    buildMiniWorld(eco);
    ContractSystem contracts(bus, eco, time);

    contracts.generateContracts(GameTime{ 0 });
    auto offers = contracts.getAvailableContracts();
    REQUIRE(offers.size() == 1);

    SUBCASE("Спрос = непокрытый дефицит") {
        // demand 40 - supply 10 = 30
        CHECK(offers[0].demandTons == doctest::Approx(30.0));
        CHECK(offers[0].destination == "city_b");
        CHECK(offers[0].origin == "city_a");
        CHECK(offers[0].active);
    }

    SUBCASE("Кризисный дефицит -> Urgent") {
        CHECK(offers[0].type == ContractType::Urgent); // deficit = 4.0 >= 2.5
    }
}

// === Multi-taker: многие берут один контракт ===

TEST_CASE("ContractSystem: multiple carriers join same contract") {
    EventBus bus;
    TimeSystem time(bus);
    EconomySystem eco(bus, EconomyMode::Living);
    buildMiniWorld(eco);
    ContractSystem contracts(bus, eco, time);
    contracts.generateContracts(GameTime{ 0 });

    auto offers = contracts.getAvailableContracts();
    REQUIRE(offers.size() == 1);
    std::string id = offers[0].id;

    int joined = 0;
    bus.subscribe<ContractJoinedEvent>([&](const ContractJoinedEvent&) { joined++; });

    CHECK(contracts.joinContract(id, makeCarrier("player", CarrierKind::Player, 10000), 10.0));
    CHECK(contracts.joinContract(id, makeCarrier("drv_a", CarrierKind::Named, 10000), 10.0));
    CHECK(contracts.joinContract(id, makeCarrier("ai_1", CarrierKind::MassAI, 10000), 10.0));

    SUBCASE("Все трое — участники, контракт всё ещё на доске") {
        CHECK(joined == 3);
        const auto* c = contracts.getContract(id);
        REQUIRE(c != nullptr);
        CHECK(c->participants.size() == 3);
        CHECK(c->active); // спрос 30, в пути 30, доставлено 0 — ещё активен
    }

    SUBCASE("Больше остатка взять нельзя") {
        CHECK_FALSE(contracts.joinContract(id, makeCarrier("ai_2", CarrierKind::MassAI, 10000), 5.0));
    }

    SUBCASE("Тягач не тянет массу") {
        CHECK_FALSE(contracts.joinContract(id, makeCarrier("ai_3", CarrierKind::MassAI, 3000), 5.0));
    }
}

// === Доставки, первая доставка, лицензия ===

TEST_CASE("ContractSystem: deliveries and first delivery license") {
    EventBus bus;
    TimeSystem time(bus);
    EconomySystem eco(bus, EconomyMode::Living);
    buildMiniWorld(eco);
    ContractSystem contracts(bus, eco, time);
    contracts.generateContracts(GameTime{ 0 });

    std::string id = contracts.getAvailableContracts()[0].id;

    // Именной стартует первым (быстрее)
    CHECK(contracts.joinContract(id, makeCarrier("drv_a", CarrierKind::Named, 10000, 60.0), 10.0));
    CHECK(contracts.joinContract(id, makeCarrier("ai_1", CarrierKind::MassAI, 10000, 40.0), 10.0));

    int arrived = 0;
    bool firstEligible = false;
    std::string firstId;
    Money totalPayout = 0;

    bus.subscribe<ContractDeliveryArrivedEvent>([&](const ContractDeliveryArrivedEvent& e) {
        arrived++;
        totalPayout += e.payout;
        CHECK(e.onTime);
        });
    bus.subscribe<ContractFirstDeliveryEvent>([&](const ContractFirstDeliveryEvent& e) {
        firstEligible = e.licenseEligible;
        firstId = e.carrierId;
        });

    contracts.resolveDeliveries(GameTime{ 60 });

    SUBCASE("Оба доставили, экономика обновилась") {
        CHECK(arrived == 2);
        CHECK(totalPayout > 0);
        CHECK(eco.getSupply("city_b", "good_food") == doctest::Approx(10.0 + 20.0));
    }

    SUBCASE("Первая доставка у именного -> лицензия положена") {
        CHECK(firstId == "drv_a");
        CHECK(firstEligible);
    }
}

TEST_CASE("ContractSystem: mass AI first -> nobody gets license") {
    EventBus bus;
    TimeSystem time(bus);
    EconomySystem eco(bus, EconomyMode::Living);
    buildMiniWorld(eco);
    ContractSystem contracts(bus, eco, time);
    contracts.generateContracts(GameTime{ 0 });

    std::string id = contracts.getAvailableContracts()[0].id;

    // Массовка быстрее
    CHECK(contracts.joinContract(id, makeCarrier("ai_1", CarrierKind::MassAI, 10000, 80.0), 10.0));
    CHECK(contracts.joinContract(id, makeCarrier("drv_a", CarrierKind::Named, 10000, 40.0), 10.0));

    bool firstEligible = true;
    std::string firstId;
    bus.subscribe<ContractFirstDeliveryEvent>([&](const ContractFirstDeliveryEvent& e) {
        firstEligible = e.licenseEligible;
        firstId = e.carrierId;
        });

    contracts.resolveDeliveries(GameTime{ 60 });

    CHECK(firstId == "ai_1");
    CHECK_FALSE(firstEligible); // массовка — лицензию никто не получает
}

// === Деактивация по покрытию спроса ===

TEST_CASE("ContractSystem: deactivation when demand covered") {
    EventBus bus;
    TimeSystem time(bus);
    EconomySystem eco(bus, EconomyMode::Living);
    buildMiniWorld(eco);
    ContractSystem contracts(bus, eco, time);
    contracts.generateContracts(GameTime{ 0 });

    std::string id = contracts.getAvailableContracts()[0].id;

    CHECK(contracts.joinContract(id, makeCarrier("drv_a", CarrierKind::Named, 20000), 20.0));
    CHECK(contracts.joinContract(id, makeCarrier("player", CarrierKind::Player, 20000), 10.0));

    bool deactivated = false;
    bus.subscribe<ContractDeactivatedEvent>([&](const ContractDeactivatedEvent&) {
        deactivated = true;
        });

    contracts.resolveDeliveries(GameTime{ 60 });

    SUBCASE("Спрос 30 покрыт -> контракт неактивен и снят с доски") {
        CHECK(deactivated);
        CHECK(contracts.getAvailableContracts().empty());
        const auto* c = contracts.getContract(id);
        REQUIRE(c != nullptr);
        CHECK_FALSE(c->active);
    }

    SUBCASE("После деактивации присоединиться нельзя") {
        CHECK_FALSE(contracts.joinContract(id, makeCarrier("ai_9", CarrierKind::MassAI, 10000), 5.0));
    }
}

// === Автогенерация и авторазрешение через время ===

TEST_CASE("ContractSystem: auto update via GameHourElapsed") {
    EventBus bus;
    TimeSystem time(bus);
    EconomySystem eco(bus, EconomyMode::Living);
    buildMiniWorld(eco);
    ContractSystem contracts(bus, eco, time);

    CHECK(contracts.joinContract("none", makeCarrier("player", CarrierKind::Player, 10000), 5.0) == false);

    // Генерация + присоединение вручную, дальше время само разрешает доставки
    contracts.generateContracts(GameTime{ 0 });
    std::string id = contracts.getAvailableContracts()[0].id;
    CHECK(contracts.joinContract(id, makeCarrier("drv_a", CarrierKind::Named, 10000), 10.0));

    int arrived = 0;
    bus.subscribe<ContractDeliveryArrivedEvent>([&](const ContractDeliveryArrivedEvent&) { arrived++; });

    // 1 игровой час = 900 реальных секунд
    time.tick(900.0);

    CHECK(arrived == 1);
}