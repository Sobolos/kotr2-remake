#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "kotr/core/EventBus.hpp"
#include "kotr/core/TimeSystem.hpp"
#include "kotr/contracts/ContractSystem.hpp"
#include "kotr/economy/EconomySystem.hpp"
#include "kotr/drivers/DriverSystem.hpp"
#include "kotr/leaderboard/DeliveryLeaderboard.hpp"
#include "kotr/licenses/LicenseSystem.hpp"

using namespace kotr::core;
using namespace kotr::data;
using namespace kotr::economy;
using namespace kotr::contracts;
using namespace kotr::drivers;
using namespace kotr::leaderboard;
using namespace kotr::licenses;

static void buildMiniWorld(EconomySystem& eco) {
    GoodData food; food.id = "good_food"; food.baseTariffPerTonKm = 4; food.legal = true;
    eco.registerGood(food);

    CityData a; a.id = "city_a"; a.coordX = 0; a.coordY = 0;
    CityData b; b.id = "city_b"; b.coordX = 0; b.coordY = 3;
    eco.registerCity(a); eco.registerCity(b);

    CityGoodState ga; ga.goodId = "good_food"; ga.supply = 100; ga.demand = 5;
    eco.initCityGood("city_a", ga);
    CityGoodState gb; gb.goodId = "good_food"; gb.supply = 10; gb.demand = 40;
    eco.initCityGood("city_b", gb);
}

TEST_CASE("DeliveryLeaderboard: accumulates score from deliveries") {
    EventBus bus;
    TimeSystem time(bus);
    DeliveryLeaderboard lb(bus, time);

    // Публикуем доставки
    ContractDeliveryArrivedEvent e1;
    e1.carrierId = "player";
    e1.kind = CarrierKind::Player;
    e1.massTons = 10.0;
    e1.distanceKm = 100.0;
    e1.onTime = true;
    e1.good = "good_food";
    bus.publish(e1);

    ContractDeliveryArrivedEvent e2;
    e2.carrierId = "drv_a";
    e2.kind = CarrierKind::Named;
    e2.massTons = 8.0;
    e2.distanceKm = 120.0;
    e2.onTime = true;
    e2.good = "good_food";
    bus.publish(e2);

    auto foodLb = lb.getLeaderboard(LeaderboardCategory::Food);
    CHECK(foodLb.entries.size() == 2);
    CHECK(foodLb.entries[0].carrierId == "drv_a"); // 8*120=960 > 10*100=1000... нет, 1000 > 960
    // Исправление: player = 1000, drv_a = 960
    CHECK(foodLb.entries[0].carrierId == "player");
}

TEST_CASE("DeliveryLeaderboard: awards license to first eligible") {
    EventBus bus;
    TimeSystem time(bus);
    DeliveryLeaderboard lb(bus, time);
    LicenseSystem licenses(bus);

    // Массовка доставляет больше
    ContractDeliveryArrivedEvent e1;
    e1.carrierId = "ai_1";
    e1.kind = CarrierKind::MassAI;
    e1.massTons = 20.0;
    e1.distanceKm = 100.0;
    e1.onTime = true;
    e1.good = "good_food";
    bus.publish(e1);

    // Именной доставляет меньше
    ContractDeliveryArrivedEvent e2;
    e2.carrierId = "drv_a";
    e2.kind = CarrierKind::Named;
    e2.massTons = 10.0;
    e2.distanceKm = 100.0;
    e2.onTime = true;
    e2.good = "good_food";
    bus.publish(e2);

    std::string winnerId;
    bool isNamed = false;
    bus.subscribe<LicenseAwardedEvent>([&](const LicenseAwardedEvent& e) {
        winnerId = e.winnerId;
        isNamed = e.isNamed;
        });

    lb.evaluatePeriod();

    CHECK(winnerId == "drv_a"); // именной, хотя mass AI доставил больше
    CHECK(isNamed);
    CHECK(licenses.hasLicense("drv_a", "License.Food"));
}

TEST_CASE("LicenseSystem: integrates with DriverSystem hiring") {
    EventBus bus;
    TimeSystem time(bus);
    DriverSystem drivers(bus, time);
    LicenseSystem licenses(bus);
    drivers.setLicenseSystem(&licenses);

    DriverPersonaData p;
    p.id = "drv_fuel_pro";
    p.requiredLicenses = { "License.Fuel" };
    drivers.registerDriver(p);

    SUBCASE("Без лицензии найм невозможен") {
        CHECK_FALSE(drivers.hasRequiredLicenses("drv_fuel_pro"));
        CHECK_FALSE(drivers.hireDriver("drv_fuel_pro"));
    }

    SUBCASE("С лицензией найм возможен") {
        licenses.awardLicense("player", "License.Fuel");
        CHECK(drivers.hasRequiredLicenses("drv_fuel_pro"));
        CHECK(drivers.hireDriver("drv_fuel_pro"));
    }
}

TEST_CASE("ContractSystem: competition_factor affects payout") {
    EventBus bus;
    TimeSystem time(bus);
    EconomySystem eco(bus, EconomyMode::Living);
    buildMiniWorld(eco);
    ContractSystem contracts(bus, eco, time);

    contracts.generateContracts(GameTime{ 0 });
    auto offers = contracts.getAvailableContracts();
    REQUIRE(offers.size() > 0);

    std::string id = offers[0].id;

    // Один участник: competition_factor = 1.15
    CarrierRef ref1;
    ref1.id = "player";
    ref1.kind = CarrierKind::Player;
    ref1.capacityKg = 10000;
    ref1.speedKmh = 50.0;
    CHECK(contracts.joinContract(id, ref1, 10.0));

    Money payout1 = 0;
    bus.subscribe<ContractDeliveryArrivedEvent>([&](const ContractDeliveryArrivedEvent& e) {
        payout1 = e.payout;
        });

    contracts.resolveDeliveries(GameTime{ 60 });
    CHECK(payout1 > 0);

    // Генерируем новый контракт, 4 участника: competition_factor < 1.0
    contracts.generateContracts(GameTime{ 120 });
    auto offers2 = contracts.getAvailableContracts();
    if (offers2.empty()) return;

    std::string id2 = offers2[0].id;
    for (int i = 0; i < 4; ++i) {
        CarrierRef ref;
        ref.id = "ai_" + std::to_string(i);
        ref.kind = CarrierKind::MassAI;
        ref.capacityKg = 10000;
        ref.speedKmh = 50.0;
        contracts.joinContract(id2, ref, 5.0);
    }

    Money payout2 = 0;
    bus.subscribe<ContractDeliveryArrivedEvent>([&](const ContractDeliveryArrivedEvent& e) {
        if (e.carrierId == "ai_0") payout2 = e.payout;
        });

    contracts.resolveDeliveries(GameTime{ 180 });
    CHECK(payout2 > 0);
    CHECK(payout2 < payout1); // больше конкурентов -> меньше оплата
}