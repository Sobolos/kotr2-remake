#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "kotr/core/EventBus.hpp"
#include "kotr/economy/EconomySystem.hpp"

using namespace kotr::core;
using namespace kotr::data;
using namespace kotr::economy;

// === Вспомогательная функция: создать базовую экономику ===

static EconomySystem createTestEconomy(EventBus& bus) {
    EconomySystem eco(bus, EconomyMode::Living);

    // Товар: Продукты питания
    GoodData food;
    food.id = "good_food";
    food.nameKey = "loc.good.food.name";
    food.category = GoodCategory::Consumer;
    food.basePricePerTon = 650;
    food.cargoValuePerTon = 650;
    food.baseTariffPerTonKm = 4;
    food.volatility = 0.2;
    food.dangerModifier = 0.1;
    food.legal = true;
    food.perishable = true;
    eco.registerGood(food);

    // Товар: Уголь
    GoodData coal;
    coal.id = "good_coal";
    coal.nameKey = "loc.good.coal.name";
    coal.category = GoodCategory::Raw;
    coal.basePricePerTon = 280;
    coal.cargoValuePerTon = 280;
    coal.baseTariffPerTonKm = 3;
    coal.volatility = 0.3;
    coal.dangerModifier = 0.3;
    coal.legal = true;
    eco.registerGood(coal);

    // Город: Шахты
    CityData shakhty;
    shakhty.id = "city_shakhty";
    shakhty.nameKey = "loc.city.shakhty.name";
    shakhty.specialization = CitySpecialization::Mining;
    shakhty.economicPower = 0.8;
    shakhty.fuelBasePrice = 10;
    shakhty.partsBaseAvailability = 0.5;
    shakhty.baseDanger = 0.65;
    eco.registerCity(shakhty);

    // Город: Ельнино
    CityData elnino;
    elnino.id = "city_elnino";
    elnino.nameKey = "loc.city.elnino.name";
    elnino.specialization = CitySpecialization::Agriculture;
    elnino.economicPower = 0.6;
    elnino.fuelBasePrice = 10;
    elnino.partsBaseAvailability = 0.6;
    elnino.baseDanger = 0.20;
    eco.registerCity(elnino);

    // Состояние товаров в Шахтах: продукты в дефиците, уголь в избытке
    CityGoodState shakhtyFood;
    shakhtyFood.goodId = "good_food";
    shakhtyFood.supply = 20.0;
    shakhtyFood.demand = 70.0;
    shakhtyFood.productionPerDay = 0.0;   // Шахты не производят продукты
    shakhtyFood.consumptionPerDay = 50.0;
    eco.initCityGood("city_shakhty", shakhtyFood);

    CityGoodState shakhtyCoal;
    shakhtyCoal.goodId = "good_coal";
    shakhtyCoal.supply = 150.0;
    shakhtyCoal.demand = 10.0;
    shakhtyCoal.productionPerDay = 80.0;
    shakhtyCoal.consumptionPerDay = 5.0;
    eco.initCityGood("city_shakhty", shakhtyCoal);

    // Состояние товаров в Ельнино: продукты в избытке
    CityGoodState elninoFood;
    elninoFood.goodId = "good_food";
    elninoFood.supply = 100.0;
    elninoFood.demand = 30.0;
    elninoFood.productionPerDay = 60.0;
    elninoFood.consumptionPerDay = 20.0;
    eco.initCityGood("city_elnino", elninoFood);

    return eco;
}

// === Тесты: Deficit Ratio ===

TEST_CASE("Economy: deficit ratio calculation") {
    EventBus bus;
    auto eco = createTestEconomy(bus);

    SUBCASE("Шахты: продукты в дефиците") {
        // demand=70, supply=20 → deficit_ratio = 70/20 = 3.5
        double ratio = eco.getDeficitRatio("city_shakhty", "good_food");
        CHECK(ratio == doctest::Approx(3.5));
    }

    SUBCASE("Шахты: уголь в избытке") {
        // demand=10, supply=150 → deficit_ratio = 10/150 ≈ 0.067
        double ratio = eco.getDeficitRatio("city_shakhty", "good_coal");
        CHECK(ratio < 0.1);
    }

    SUBCASE("Ельнино: продукты в норме") {
        // demand=30, supply=100 → deficit_ratio = 30/100 = 0.3
        double ratio = eco.getDeficitRatio("city_elnino", "good_food");
        CHECK(ratio == doctest::Approx(0.3));
    }
}

// === Тесты: Price Factor ===

TEST_CASE("Economy: price factor") {
    EventBus bus;
    auto eco = createTestEconomy(bus);

    SUBCASE("Высокий дефицит → высокий price factor") {
        // deficit_ratio = 3.5 → price_factor = 0.6 + 0.4*3.5 = 2.0
        // clamp(2.0, 0.5, 3.0) = 2.0
        // price = 650 * 2.0 = 1300
        Money price = eco.getPrice("city_shakhty", "good_food");
        CHECK(price == 1300);
    }

    SUBCASE("Избыток → низкий price factor") {
        // deficit_ratio ≈ 0.067 → price_factor = 0.6 + 0.4*0.067 ≈ 0.627
        // price = 280 * 0.627 ≈ 176
        Money price = eco.getPrice("city_shakhty", "good_coal");
        CHECK(price >= 160);
        CHECK(price <= 180);
    }

    SUBCASE("Нормальный дефицит → базовый price factor") {
        // deficit_ratio = 0.3 → price_factor = 0.6 + 0.4*0.3 = 0.72
        // price = 650 * 0.72 = 468
        Money price = eco.getPrice("city_elnino", "good_food");
        CHECK(price == doctest::Approx(468).epsilon(1.0));
    }
}

// === Тесты: Пределы цен ===

TEST_CASE("Economy: price clamping") {
    EventBus bus;
    auto eco = createTestEconomy(bus);

    SUBCASE("Цена не ниже base_price * 0.4") {
        // Даже при огромном избытке
        // min_price = 650 * 0.4 = 260
        // Если price_factor < 0.4, цена всё равно 260
        Money minPrice = 650 * 4 / 10; // 260
        Money price = eco.getPrice("city_elnino", "good_food");
        CHECK(price >= minPrice);
    }

    SUBCASE("Цена не выше base_price * 4.0") {
        // max_price = 650 * 4.0 = 2600
        Money maxPrice = 650 * 4;
        Money price = eco.getPrice("city_shakhty", "good_food");
        CHECK(price <= maxPrice);
    }
}

// === Тесты: Доставка товара ===

TEST_CASE("Economy: delivery affects supply and demand") {
    EventBus bus;
    auto eco = createTestEconomy(bus);

    // До доставки: supply=20, demand=70
    CHECK(eco.getSupply("city_shakhty", "good_food") == doctest::Approx(20.0));
    CHECK(eco.getDemand("city_shakhty", "good_food") == doctest::Approx(70.0));

    // Доставляем 30 тонн продуктов в Шахты
    eco.deliverGood("city_shakhty", "good_food", 30.0);

    CHECK(eco.getSupply("city_shakhty", "good_food") == doctest::Approx(50.0));
    CHECK(eco.getDemand("city_shakhty", "good_food") == doctest::Approx(40.0));

    // Дефицит снизился
    double ratio = eco.getDeficitRatio("city_shakhty", "good_food");
    CHECK(ratio == doctest::Approx(40.0 / 50.0));
}

// === Тесты: Производство и потребление ===

TEST_CASE("Economy: hourly production and consumption") {
    EventBus bus;
    auto eco = createTestEconomy(bus);

    // Ельнино: production=60/day, consumption=20/day
    double initialSupply = eco.getSupply("city_elnino", "good_food");

    // Обновляем 24 часа (1 день)
    for (int i = 0; i < 24; ++i) {
        eco.updateHour();
    }

    double newSupply = eco.getSupply("city_elnino", "good_food");

    // За день: supply += production - consumption = 60 - 20 = +40
    CHECK(newSupply == doctest::Approx(initialSupply + 40.0).epsilon(1.0));
}

// === Тесты: События ===

TEST_CASE("Economy: price change events") {
    EventBus bus;
    auto eco = createTestEconomy(bus);

    int priceChangeCount = 0;
    bus.subscribe<PriceChangedEvent>([&](const PriceChangedEvent& e) {
        priceChangeCount++;
        });

    // Обновляем экономику
    eco.updateHour();

    // Должны быть события изменения цен
    CHECK(priceChangeCount > 0);
}

// === Тесты: Classic vs Living ===

TEST_CASE("Economy: classic mode prevents supply drop") {
    EventBus bus;
    auto eco = createTestEconomy(bus);
    eco.setMode(EconomyMode::Classic);

    // В Classic режиме supply не падает ниже 50% от demand
    eco.consumeGood("city_shakhty", "good_food", 100.0);
    eco.updateHour();

    double supply = eco.getSupply("city_shakhty", "good_food");
    double demand = eco.getDemand("city_shakhty", "good_food");

    // В Classic: supply >= demand * 0.5
    CHECK(supply >= demand * 0.5);
}