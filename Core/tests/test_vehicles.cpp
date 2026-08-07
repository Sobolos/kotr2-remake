#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "kotr/core/EventBus.hpp"
#include "kotr/core/TimeSystem.hpp"
#include "kotr/economy/EconomySystem.hpp"
#include "kotr/contracts/ContractSystem.hpp"
#include "kotr/drivers/DriverSystem.hpp"
#include "kotr/drivers/DriverAgentSystem.hpp"
#include "kotr/vechicles/VechicleSystem.hpp"

using namespace kotr::core;
using namespace kotr::data;
using namespace kotr::economy;
using namespace kotr::contracts;
using namespace kotr::drivers;
using namespace kotr::vehicles;

static VehicleModelData makeZil() {
    VehicleModelData m;
    m.id = "veh_model_zil130"; m.vehicleClass = VehicleClass::Light;
    m.marketPayloadKg = 5000; m.massKg = 4300; m.enginePowerKw = 90;
    m.topSpeedKmh = 80; m.reliability = 0.45;
    return m;
}
static VehicleModelData makeKamaz() {
    VehicleModelData m;
    m.id = "veh_model_kamaz5320"; m.vehicleClass = VehicleClass::Medium;
    m.marketPayloadKg = 10000; m.massKg = 9000; m.enginePowerKw = 210;
    m.topSpeedKmh = 90; m.reliability = 0.6;
    return m;
}
static VehicleModelData makeScania() {
    VehicleModelData m;
    m.id = "veh_model_scania113"; m.vehicleClass = VehicleClass::Heavy;
    m.marketPayloadKg = 20000; m.massKg = 12000; m.enginePowerKw = 380;
    m.topSpeedKmh = 105; m.reliability = 0.85;
    return m;
}

TEST_CASE("VehicleSystem: speed hierarchy by class (баланс §33)") {
    VehicleSystem vs;
    vs.registerModel(makeZil());
    vs.registerModel(makeKamaz());
    vs.registerModel(makeScania());

    auto zil = vs.createInstance("veh_model_zil130", "player", 100);
    auto kam = vs.createInstance("veh_model_kamaz5320", "player", 100);
    auto sca = vs.createInstance("veh_model_scania113", "player", 100);

    double vZil = vs.calcEffectiveSpeedKmh(zil, 5000);
    double vKam = vs.calcEffectiveSpeedKmh(kam, 10000);
    double vSca = vs.calcEffectiveSpeedKmh(sca, 20000);

    SUBCASE("Попадают в целевые диапазоны") {
        CHECK(vZil >= 60.0); CHECK(vZil <= 75.0);   // ранний
        CHECK(vKam >= 75.0); CHECK(vKam <= 90.0);   // средний
        CHECK(vSca >= 85.0);                        // поздний
    }
    SUBCASE("Иерархия классов") {
        CHECK(vZil < vKam);
        CHECK(vKam < vSca);
    }
}

TEST_CASE("VehicleSystem: condition, cargo, road limit, upgrades") {
    VehicleSystem vs;
    vs.registerModel(makeKamaz());
    auto kam = vs.createInstance("veh_model_kamaz5320", "player", 100);

    SUBCASE("Износ снижает скорость") {
        double fresh = vs.calcEffectiveSpeedKmh(kam, 10000);
        vs.setCondition(kam, 20);
        double worn = vs.calcEffectiveSpeedKmh(kam, 10000);
        CHECK(worn < fresh);
    }

    SUBCASE("Гружёный едет медленнее пустого") {
        CHECK(vs.calcEffectiveSpeedKmh(kam, 0) > vs.calcEffectiveSpeedKmh(kam, 10000));
    }

    SUBCASE("Лимит дороги ограничивает") {
        CHECK(vs.calcEffectiveSpeedKmh(kam, 0, 60.0) <= 60.0);
    }

    SUBCASE("Апгрейд ускоряет") {
        double before = vs.calcEffectiveSpeedKmh(kam, 10000);
        vs.applyUpgrade(kam, "Upgrade.Engine.Turbo");
        double after = vs.calcEffectiveSpeedKmh(kam, 10000);
        CHECK(after == doctest::Approx(before * 1.08));
    }

    SUBCASE("Рыночная ёмкость = payload тягача") {
        CHECK(vs.getMarketCapacityKg(kam) == doctest::Approx(10000.0));
    }
}

// Интеграция: агенты едут на своих машинах
TEST_CASE("DriverAgentSystem: agents drive their own trucks") {
    EventBus bus;
    TimeSystem time(bus);
    EconomySystem eco(bus, EconomyMode::Living);

    // Дальний маршрут, чтобы разница скоростей была заметна
    GoodData food; food.id = "good_food"; food.baseTariffPerTonKm = 4; food.legal = true;
    eco.registerGood(food);
    CityData a; a.id = "city_a"; a.coordX = 0; a.coordY = 0;
    CityData b; b.id = "city_b"; b.coordX = 0; b.coordY = 30;
    eco.registerCity(a); eco.registerCity(b);
    CityGoodState ga; ga.goodId = "good_food"; ga.supply = 100; ga.demand = 5;
    eco.initCityGood("city_a", ga);
    CityGoodState gb; gb.goodId = "good_food"; gb.supply = 10; gb.demand = 40;
    eco.initCityGood("city_b", gb);

    ContractSystem contracts(bus, eco, time);
    DriverSystem drivers(bus, time);
    DriverAgentSystem agents(bus, eco, contracts, drivers, time);
    agents.setBaseTakeRate(1.0);

    VehicleSystem vs;
    vs.registerModel(makeZil());
    vs.registerModel(makeScania());
    auto zilVeh = vs.createInstance("veh_model_zil130", "ai_slow", 100);
    auto scaVeh = vs.createInstance("veh_model_scania113", "ai_fast", 100);
    agents.setVehicleSystem(&vs);

    agents.addMassAI("ai_slow", 5000, 1.0, "city_a");
    agents.addMassAI("ai_fast", 20000, 1.0, "city_a");
    agents.assignVehicle("ai_slow", zilVeh);
    agents.assignVehicle("ai_fast", scaVeh);

    SUBCASE("Ёмкость агента синхронизирована с тягачом") {
        CHECK(agents.getAgent("ai_slow")->capacityKg == doctest::Approx(5000.0));
        CHECK(agents.getAgent("ai_fast")->capacityKg == doctest::Approx(20000.0));
    }

    contracts.generateContracts(GameTime{ 0 });
    agents.updateHour(GameTime{ 0 });

    // Scania (~90 км/ч) доедет за ~26 мин, ЗиЛ (~64 км/ч) за ~36 мин
    contracts.resolveDeliveries(GameTime{ 30 });

    SUBCASE("Быстрый тягач доставляет раньше") {
        CHECK(agents.getAgent("ai_fast")->busy == false);
        CHECK(agents.getAgent("ai_slow")->busy == true);
        CHECK(vs.getInstance(scaVeh)->cargoLoadKg == doctest::Approx(0.0));   // разгрузился
        CHECK(vs.getInstance(zilVeh)->cargoLoadKg == doctest::Approx(5000.0)); // ещё везёт
    }

    contracts.resolveDeliveries(GameTime{ 60 });
    CHECK(agents.getAgent("ai_slow")->busy == false);
}