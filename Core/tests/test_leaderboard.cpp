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

TEST_CASE("DeliveryLeaderboard: accumulates score from deliveries") {
    EventBus bus;
    TimeSystem time(bus);
    DeliveryLeaderboard lb(bus, time);

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

    auto lb_data = lb.getLeaderboard();
    REQUIRE(lb_data.entries.size() == 2);
    CHECK(lb_data.entries[0].carrierId == "player");
    CHECK(lb_data.entries[1].carrierId == "drv_a");
    CHECK(lb_data.entries[0].score == doctest::Approx(1000.0));
}

TEST_CASE("DeliveryLeaderboard: late delivery penalises score") {
    EventBus bus;
    TimeSystem time(bus);
    DeliveryLeaderboard lb(bus, time);

    ContractDeliveryArrivedEvent e;
    e.carrierId = "player";
    e.kind = CarrierKind::Player;
    e.massTons = 10.0;
    e.distanceKm = 100.0;
    e.onTime = false;
    bus.publish(e);

    auto lb_data = lb.getLeaderboard();
    CHECK(lb_data.entries[0].score == doctest::Approx(700.0));
}

TEST_CASE("DeliveryLeaderboard: awards license to first eligible, not massAI") {
    EventBus bus;
    TimeSystem time(bus);
    DeliveryLeaderboard lb(bus, time);
    LicenseSystem licenses(bus);

    ContractDeliveryArrivedEvent e1;
    e1.carrierId = "ai_1";
    e1.kind = CarrierKind::MassAI;
    e1.massTons = 20.0;
    e1.distanceKm = 100.0;
    e1.onTime = true;
    bus.publish(e1);

    ContractDeliveryArrivedEvent e2;
    e2.carrierId = "drv_a";
    e2.kind = CarrierKind::Named;
    e2.massTons = 10.0;
    e2.distanceKm = 100.0;
    e2.onTime = true;
    bus.publish(e2);

    std::string winnerId;
    bool isNamed = false;
    int awardedCount = 0;
    bus.subscribe<LicenseAwardedEvent>([&](const LicenseAwardedEvent& e) {
        winnerId = e.winnerId;
        isNamed = e.isNamed;
        awardedCount = e.licenseCount;
        });

    lb.evaluatePeriod();

    CHECK(winnerId == "drv_a");
    CHECK(isNamed);
    CHECK(awardedCount == 1);
    CHECK(licenses.getLicenseCount("drv_a") == 1);
    CHECK(licenses.getLicenseCount("ai_1") == 0);
}

TEST_CASE("DeliveryLeaderboard: streaks grant multiple licenses") {
    EventBus bus;
    TimeSystem time(bus);
    DeliveryLeaderboard lb(bus, time);
    LicenseSystem licenses(bus);

    std::string lastWinner;
    int lastAwarded = 0;
    bus.subscribe<LicenseAwardedEvent>([&](const LicenseAwardedEvent& e) {
        lastWinner = e.winnerId;
        lastAwarded = e.licenseCount;
        });

    auto deliver = [&](const std::string& carrierId, CarrierKind kind) {
        ContractDeliveryArrivedEvent e;
        e.carrierId = carrierId;
        e.kind = kind;
        e.massTons = 10.0;
        e.distanceKm = 100.0;
        e.onTime = true;
        bus.publish(e);
        };

    SUBCASE("1 win -> 1 license") {
        deliver("drv_a", CarrierKind::Named);
        lb.evaluatePeriod();
        CHECK(lastWinner == "drv_a");
        CHECK(lastAwarded == 1);
        CHECK(licenses.getLicenseCount("drv_a") == 1);
    }

    SUBCASE("2 consecutive wins -> 2 licenses") {
        deliver("drv_a", CarrierKind::Named);
        lb.evaluatePeriod();
        CHECK(lastAwarded == 1);

        deliver("drv_a", CarrierKind::Named);
        lb.evaluatePeriod();
        CHECK(lastWinner == "drv_a");
        CHECK(lastAwarded == 2);
        CHECK(licenses.getLicenseCount("drv_a") == 3);
    }

    SUBCASE("3+ consecutive wins -> 3 licenses (cap)") {
        deliver("drv_a", CarrierKind::Named); lb.evaluatePeriod();
        deliver("drv_a", CarrierKind::Named); lb.evaluatePeriod();
        deliver("drv_a", CarrierKind::Named); lb.evaluatePeriod();
        CHECK(lastAwarded == 3);

        deliver("drv_a", CarrierKind::Named); lb.evaluatePeriod();
        CHECK(lastAwarded == 3);
    }

    SUBCASE("winner change resets streak") {
        deliver("drv_a", CarrierKind::Named); lb.evaluatePeriod();
        deliver("drv_b", CarrierKind::Named); lb.evaluatePeriod();
        CHECK(lastWinner == "drv_b");
        CHECK(lastAwarded == 1);

        deliver("drv_a", CarrierKind::Named); lb.evaluatePeriod();
        CHECK(lastWinner == "drv_a");
        CHECK(lastAwarded == 1);
    }
}

TEST_CASE("DeliveryLeaderboard: empty period -> no license awarded") {
    EventBus bus;
    TimeSystem time(bus);
    DeliveryLeaderboard lb(bus, time);

    int events = 0;
    bus.subscribe<LicenseAwardedEvent>([&](const LicenseAwardedEvent&) { events++; });

    lb.evaluatePeriod();
    CHECK(events == 0);
}

TEST_CASE("LicenseSystem: award and revoke counts") {
    EventBus bus;
    LicenseSystem ls(bus);

    ls.awardLicenses("player", 2);
    CHECK(ls.getLicenseCount("player") == 2);

    ls.awardLicenses("player", 1);
    CHECK(ls.getLicenseCount("player") == 3);

    ls.revokeLicenses("player", 1);
    CHECK(ls.getLicenseCount("player") == 2);

    ls.revokeLicenses("player", 10);
    CHECK(ls.getLicenseCount("player") == 0);

    CHECK(ls.hasLicenses("player", 0));
    CHECK_FALSE(ls.hasLicenses("player", 1));
    CHECK_FALSE(ls.hasLicenses("unknown", 1));
}

TEST_CASE("LicenseSystem: integration with DriverSystem hiring") {
    EventBus bus;
    TimeSystem time(bus);
    DriverSystem drivers(bus, time);
    LicenseSystem licenses(bus);
    drivers.setLicenseSystem(&licenses);

    drivers.registerDriver(makePersona("drv_ivan"));

    SUBCASE("Без лицензий найм невозможен") {
        CHECK(licenses.getLicenseCount("player") == 0);
        CHECK_FALSE(drivers.canHireMoreDrivers("player"));
        CHECK_FALSE(drivers.hireDriver("drv_ivan"));
    }

    SUBCASE("С 1 лицензией найм возможен") {
        licenses.awardLicenses("player", 1);
        CHECK(drivers.canHireMoreDrivers("player"));
        CHECK(drivers.hireDriver("drv_ivan"));
    }
}