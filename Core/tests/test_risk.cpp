#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "kotr/risk/RiskSystem.hpp"
#include "kotr/core/EventBus.hpp"
#include "kotr/core/TimeSystem.hpp"
#include "kotr/economy/EconomySystem.hpp"
#include "kotr/contracts/ContractSystem.hpp"
#include "kotr/radio/RadioSystem.hpp"

using namespace kotr::core;
using namespace kotr::risk;
using namespace kotr::economy;
using namespace kotr::contracts;
using namespace kotr::radio;
using namespace kotr::data;

static void setupMiniWorld(EconomySystem& eco) {
    GoodData food;
    food.id = "good_food";
    food.baseTariffPerTonKm = 4;
    food.legal = true;
    food.dangerModifier = 0.0;
    eco.registerGood(food);

    CityData a; a.id = "city_a"; a.coordX = 0; a.coordY = 0; a.baseDanger = 0.1;
    CityData b; b.id = "city_b"; b.coordX = 0; b.coordY = 3; b.baseDanger = 0.1;
    eco.registerCity(a); eco.registerCity(b);

    CityGoodState ga; ga.goodId = "good_food"; ga.supply = 100; ga.demand = 5;
    eco.initCityGood("city_a", ga);
    CityGoodState gb; gb.goodId = "good_food"; gb.supply = 10; gb.demand = 40;
    eco.initCityGood("city_b", gb);
}

TEST_CASE("RiskSystem: initial state") {
    EventBus bus;
    TimeSystem time(bus);
    RiskSystem risk(bus, time);

    CHECK(risk.getPoliceHeat() == 0);
    CHECK(risk.getBanditPressure() == 0);
    CHECK(risk.getCurrentThreatLevel() == -1);
}

TEST_CASE("RiskSystem: add and clamp heat values") {
    EventBus bus;
    TimeSystem time(bus);
    RiskSystem risk(bus, time);

    risk.addPoliceHeat(30);
    CHECK(risk.getPoliceHeat() == 30);

    risk.addPoliceHeat(200);
    CHECK(risk.getPoliceHeat() == 100);

    risk.addPoliceHeat(-50);
    CHECK(risk.getPoliceHeat() == 50);

    risk.addBanditPressure(40);
    CHECK(risk.getBanditPressure() == 40);

    risk.addBanditPressure(-100);
    CHECK(risk.getBanditPressure() == 0);
}

TEST_CASE("RiskSystem: decay on updateHour") {
    EventBus bus;
    TimeSystem time(bus);
    RiskSystem risk(bus, time);

    risk.setPoliceHeat(50);
    risk.setBanditPressure(30);

    risk.updateHour(GameTime{ 60 });
    CHECK(risk.getPoliceHeat() == 45);
    CHECK(risk.getBanditPressure() == 28);

    risk.setPoliceHeat(3);
    risk.setBanditPressure(1);
    risk.updateHour(GameTime{ 120 });
    CHECK(risk.getPoliceHeat() == 0);
    CHECK(risk.getBanditPressure() == 0);
}

TEST_CASE("RiskSystem: threat thresholds") {
    EventBus bus;
    TimeSystem time(bus);
    RiskSystem risk(bus, time);

    int lastLevel = -999;
    std::string lastName;
    bus.subscribe<ThreatLevelChangedEvent>([&](const ThreatLevelChangedEvent& e) {
        lastLevel = e.newThreatLevel;
        lastName = e.threatName;
        });

    SUBCASE("Heat 15 -> Low") {
        risk.setPoliceHeat(15);
        CHECK(risk.getCurrentThreatLevel() == 0);
        CHECK(lastLevel == 0);
        CHECK(lastName == "Low");
    }

    SUBCASE("Heat 35 -> Medium") {
        risk.setBanditPressure(35);
        CHECK(risk.getCurrentThreatLevel() == 1);
        CHECK(lastLevel == 1);
    }

    SUBCASE("Heat 60 -> High") {
        risk.setPoliceHeat(60);
        CHECK(risk.getCurrentThreatLevel() == 2);
        CHECK(lastLevel == 2);
    }

    SUBCASE("Heat 90 -> Extreme") {
        risk.setPoliceHeat(90);
        CHECK(risk.getCurrentThreatLevel() == 3);
        CHECK(lastLevel == 3);
        CHECK(lastName == "Extreme");
    }

    SUBCASE("max of police and bandit is used") {
        risk.setPoliceHeat(20);     // Low (0)
        risk.setBanditPressure(65); // High (2), т.к. 65 >= 60
        CHECK(risk.getCurrentThreatLevel() == 2);
    }
}

TEST_CASE("RiskSystem: event only fires on level change") {
    EventBus bus;
    TimeSystem time(bus);
    RiskSystem risk(bus, time);

    int events = 0;
    bus.subscribe<ThreatLevelChangedEvent>([&](const ThreatLevelChangedEvent&) { events++; });

    risk.setPoliceHeat(10);  // ниже порога Low(15) — всё ещё Safe
    CHECK(events == 0);

    risk.setPoliceHeat(16);  // Low
    CHECK(events == 1);

    risk.setPoliceHeat(25);  // всё ещё Low
    CHECK(events == 1);

    risk.setPoliceHeat(40);  // Medium
    CHECK(events == 2);
}

TEST_CASE("RiskSystem: route risk multiplier formula") {
    EventBus bus;
    TimeSystem time(bus);
    RiskSystem risk(bus, time);

    CHECK(risk.getRouteRiskMultiplier() == doctest::Approx(1.0));

    risk.setPoliceHeat(100);
    risk.setBanditPressure(0);
    CHECK(risk.getRouteRiskMultiplier() == doctest::Approx(1.35));

    risk.setPoliceHeat(50);
    risk.setBanditPressure(50);
    CHECK(risk.getRouteRiskMultiplier() == doctest::Approx(1.175));
}

TEST_CASE("RiskSystem: integration with ContractSystem via deliveries") {
    EventBus bus;
    TimeSystem time(bus);
    EconomySystem eco(bus, EconomyMode::Living);
    setupMiniWorld(eco);
    ContractSystem contracts(bus, eco, time);
    RiskSystem risk(bus, time);
    contracts.setRiskSystem(&risk);

    SUBCASE("High risk increases payout vs low risk (isolated worlds)") {
        // --- Мир 1: нулевой риск ---
        Money payoutLow = 0;
        {
            EventBus bus1;
            TimeSystem time1(bus1);
            EconomySystem eco1(bus1, EconomyMode::Living);
            setupMiniWorld(eco1);
            ContractSystem contracts1(bus1, eco1, time1);
            RiskSystem risk1(bus1, time1);
            contracts1.setRiskSystem(&risk1);

            contracts1.generateContracts(GameTime{ 0 });
            auto offers = contracts1.getAvailableContracts();
            REQUIRE_FALSE(offers.empty());

            bus1.subscribe<ContractDeliveryArrivedEvent>([&](const ContractDeliveryArrivedEvent& e) {
                payoutLow = e.payout;
                });

            CarrierRef ref;
            ref.id = "player";
            ref.kind = CarrierKind::Player;
            ref.capacityKg = 10000;
            ref.speedKmh = 50.0;
            REQUIRE(contracts1.joinContract(offers[0].id, ref, 5.0));
            contracts1.resolveDeliveries(GameTime{ 60 });
        }
        REQUIRE(payoutLow > 0);

        // --- Мир 2: высокий риск, та же начальная экономика ---
        Money payoutHigh = 0;
        {
            EventBus bus2;
            TimeSystem time2(bus2);
            EconomySystem eco2(bus2, EconomyMode::Living);
            setupMiniWorld(eco2); // та же начальная экономика
            ContractSystem contracts2(bus2, eco2, time2);
            RiskSystem risk2(bus2, time2);
            contracts2.setRiskSystem(&risk2);

            // Устанавливаем высокий риск ДО генерации и доставки
            risk2.setPoliceHeat(100);
            risk2.setBanditPressure(100);

            contracts2.generateContracts(GameTime{ 0 });
            auto offers = contracts2.getAvailableContracts();
            REQUIRE_FALSE(offers.empty());

            bus2.subscribe<ContractDeliveryArrivedEvent>([&](const ContractDeliveryArrivedEvent& e) {
                payoutHigh = e.payout;
                });

            CarrierRef ref;
            ref.id = "player";
            ref.kind = CarrierKind::Player;
            ref.capacityKg = 10000;
            ref.speedKmh = 50.0;
            REQUIRE(contracts2.joinContract(offers[0].id, ref, 5.0));
            contracts2.resolveDeliveries(GameTime{ 60 });
        }
        REQUIRE(payoutHigh > 0);

        // Высокий риск → выше route_risk → выше payout
        CHECK(payoutHigh > payoutLow);
    }
}

TEST_CASE("RadioSystem: plays announcement on threat change") {
    EventBus bus;
    TimeSystem time(bus);
    RiskSystem risk(bus, time);
    RadioSystem radio(bus);

    std::string lastMsg = "";
    bus.subscribe<RadioAnnouncementEvent>([&](const RadioAnnouncementEvent& e) {
        lastMsg = e.messageKey;
        });

    SUBCASE("Heat below threshold -> no event") {
        risk.setPoliceHeat(10); // < 15, Safe
        CHECK(lastMsg == "");   // ничего не публикуется
    }

    SUBCASE("Heat 15 -> Low announcement") {
        risk.setPoliceHeat(15);
        CHECK(lastMsg == "radio.danger.low");
    }

    SUBCASE("Heat 40 -> Medium announcement") {
        risk.setPoliceHeat(40);
        CHECK(lastMsg == "radio.danger.medium");
    }

    SUBCASE("Heat 95 -> Extreme announcement") {
        risk.setPoliceHeat(95);
        CHECK(lastMsg == "radio.danger.extreme");
    }
}