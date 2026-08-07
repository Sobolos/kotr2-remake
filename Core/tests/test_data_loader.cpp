#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <fstream>
#include <sstream>

#include "kotr/data/DataLoader.hpp"
#include "kotr/economy/EconomySystem.hpp"
#include <nlohmann/json.hpp>

using namespace kotr::core;
using namespace kotr::data;
using namespace kotr::economy;

// === Вспомогательная функция: загрузить JSON из файла ===

static std::string readDataFile(const std::string& filename) {
    // Пробуем несколько путей (для разных конфигураций сборки)
    std::vector<std::string> paths = {
        "Core/data/" + filename,
        "../Core/data/" + filename,
        "../../Core/data/" + filename,
        "../../../Core/data/" + filename,
        "data/" + filename
    };

    for (const auto& path : paths) {
        std::ifstream file(path);
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            return buffer.str();
        }
    }
    return ""; // Файл не найден
}

// === Тесты: Загрузка товаров ===

TEST_CASE("DataLoader: load goods from JSON") {
    std::string goodsJson = readDataFile("goods.json");
    if (goodsJson.empty()) {
        MESSAGE("goods.json not found, skipping file test");
        return;
    }

    std::string citiesJson = readDataFile("cities.json");
    if (citiesJson.empty()) {
        MESSAGE("cities.json not found, skipping file test");
        return;
    }

    WorldData world = DataLoader::loadFromStrings(goodsJson, citiesJson);

    SUBCASE("Все 9 товаров загружены") {
        CHECK(world.goods.size() == 9);
    }

    SUBCASE("Проверка данных продуктов") {
        const GoodData* food = nullptr;
        for (const auto& g : world.goods) {
            if (g.id == "good_food") { food = &g; break; }
        }
        REQUIRE(food != nullptr);
        CHECK(food->basePricePerTon == 650);
        CHECK(food->baseTariffPerTonKm == 4);
        CHECK(food->category == GoodCategory::Consumer);
        CHECK(food->perishable == true);
        CHECK(food->legal == true);
    }

    SUBCASE("Проверка данных алмазов") {
        const GoodData* gems = nullptr;
        for (const auto& g : world.goods) {
            if (g.id == "good_gems") { gems = &g; break; }
        }
        REQUIRE(gems != nullptr);
        CHECK(gems->basePricePerTon == 12000);
        CHECK(gems->dangerModifier == doctest::Approx(0.9));
        CHECK(gems->category == GoodCategory::Special);
    }
}

// === Тесты: Загрузка городов ===

TEST_CASE("DataLoader: load cities from JSON") {
    std::string goodsJson = readDataFile("goods.json");
    std::string citiesJson = readDataFile("cities.json");
    if (goodsJson.empty() || citiesJson.empty()) {
        MESSAGE("Data files not found, skipping");
        return;
    }

    WorldData world = DataLoader::loadFromStrings(goodsJson, citiesJson);

    SUBCASE("Все 11 городов загружены") {
        CHECK(world.cities.size() == 11);
    }

    SUBCASE("Проверка координат Южного") {
        const CityData* yuzhny = nullptr;
        for (const auto& c : world.cities) {
            if (c.id == "city_yuzhny") { yuzhny = &c; break; }
        }
        REQUIRE(yuzhny != nullptr);
        CHECK(yuzhny->coordX == doctest::Approx(4.5));
        CHECK(yuzhny->coordY == doctest::Approx(3.0));
        CHECK(yuzhny->baseDanger == doctest::Approx(0.35));
        CHECK(yuzhny->specialization == CitySpecialization::Logistics);
    }

    SUBCASE("Проверка опасности Шахт") {
        const CityData* shakhty = nullptr;
        for (const auto& c : world.cities) {
            if (c.id == "city_shakhty") { shakhty = &c; break; }
        }
        REQUIRE(shakhty != nullptr);
        CHECK(shakhty->baseDanger == doctest::Approx(0.65));
        CHECK(shakhty->specialization == CitySpecialization::Mining);
    }

    SUBCASE("Товары городов загружены") {
        // 11 городов × 9 товаров = 99 записей
        CHECK(world.cityGoods.size() == 99);
    }
}

// === Тесты: Полная инициализация экономики ===

TEST_CASE("DataLoader: full economy initialization") {
    std::string goodsJson = readDataFile("goods.json");
    std::string citiesJson = readDataFile("cities.json");
    if (goodsJson.empty() || citiesJson.empty()) {
        MESSAGE("Data files not found, skipping");
        return;
    }

    WorldData world = DataLoader::loadFromStrings(goodsJson, citiesJson);
    EventBus bus;
    EconomySystem eco(bus, EconomyMode::Living);

    // Регистрируем товары
    for (const auto& good : world.goods) {
        eco.registerGood(good);
    }

    // Регистрируем города
    for (const auto& city : world.cities) {
        eco.registerCity(city);
    }

    // Инициализируем товары в городах
    // Нужно сопоставить cityGoods с городами
    // В loadFromStrings мы храним cityGoods в порядке городов
    // Нужно знать, к какому городу относится каждый CityGoodState
    // Для этого пересоздадим логику: парсим города заново
    // Упрощение: инициализируем напрямую из JSON

    // Альтернативный подход: загружаем и инициализируем в одном цикле
    auto citiesRoot = nlohmann::json::parse(citiesJson);
    for (const auto& cityItem : citiesRoot["cities"]) {
        CityId cityId = cityItem["id"].get<std::string>();
        if (cityItem.contains("goods")) {
            for (const auto& goodItem : cityItem["goods"]) {
                CityGoodState state;
                state.goodId = goodItem["good_id"].get<std::string>();
                state.supply = goodItem.value("initial_supply", 0.0);
                state.demand = goodItem.value("initial_demand", 0.0);
                state.productionPerDay = goodItem.value("production_per_day", 0.0);
                state.consumptionPerDay = goodItem.value("consumption_per_day", 0.0);
                eco.initCityGood(cityId, state);
            }
        }
    }

    SUBCASE("Экономика инициализирована") {
        CHECK(eco.getCityState("city_yuzhny") != nullptr);
        CHECK(eco.getCityState("city_shakhty") != nullptr);
        CHECK(eco.getCityState("city_almazny") != nullptr);
    }

    SUBCASE("Шахты: уголь в избытке, продукты в дефиците") {
        // Уголь: supply=120, demand=3 → deficit_ratio ≈ 0.025
        double coalDeficit = eco.getDeficitRatio("city_shakhty", "good_coal");
        CHECK(coalDeficit < 0.1);

        // Продукты: supply=15, demand=35 → deficit_ratio ≈ 2.33
        double foodDeficit = eco.getDeficitRatio("city_shakhty", "good_food");
        CHECK(foodDeficit > 1.5);
    }

    SUBCASE("Приозёрск: топливо в избытке") {
        // Топливо: supply=120, demand=20 → deficit_ratio ≈ 0.17
        double fuelDeficit = eco.getDeficitRatio("city_priozersk", "good_fuel");
        CHECK(fuelDeficit < 0.5);
    }

    SUBCASE("Цены инициализированы (не ноль)") {
        Money shakhtyFoodPrice = eco.getPrice("city_shakhty", "good_food");
        CHECK(shakhtyFoodPrice > 0);

        Money priozerskFuelPrice = eco.getPrice("city_priozersk", "good_fuel");
        CHECK(priozerskFuelPrice > 0);
    }

    SUBCASE("Симуляция одного дня не ломает экономику") {
        // Запускаем 24 часа
        for (int i = 0; i < 24; ++i) {
            eco.updateHour();
        }

        // Экономика всё ещё работает
        CHECK(eco.getCityState("city_shakhty") != nullptr);

        // Продукты в Шахтах: потребление 45/день, производство 0
        // За день demand вырос, supply не изменился (кроме доставок)
        double foodSupply = eco.getSupply("city_shakhty", "good_food");
        double foodDemand = eco.getDemand("city_shakhty", "good_food");
        CHECK(foodDemand > foodSupply); // Дефицит усилился
    }
}

// === Тесты: Загрузка из строк (без файлов) ===

TEST_CASE("DataLoader: load from embedded JSON strings") {
    // Минимальный JSON для теста без зависимости от файлов
    std::string goodsJson = R"({
        "version": "0.1",
        "goods": [
            {
                "id": "good_food",
                "name_key": "loc.good.food.name",
                "category": "consumer",
                "storage_class": "perishable",
                "base_price_per_ton": 650,
                "cargo_value_per_ton": 650,
                "base_tariff_per_ton_km": 4,
                "volatility": 0.15,
                "danger_modifier": 0.1,
                "legal": true,
                "perishable": true,
                "tags": ["Cargo.Food"]
            }
        ]
    })";

    std::string citiesJson = R"({
        "version": "0.1",
        "cities": [
            {
                "id": "city_test",
                "name_key": "loc.city.test.name",
                "specialization": "logistics",
                "economic_power": 1.0,
                "storage_capacity_multiplier": 1.0,
                "fuel_base_price": 10,
                "parts_base_availability": 0.7,
                "base_danger": 0.3,
                "coord_x": 5.0,
                "coord_y": 5.0,
                "goods": [
                    {
                        "good_id": "good_food",
                        "production_per_day": 10,
                        "consumption_per_day": 30,
                        "initial_supply": 15,
                        "initial_demand": 25
                    }
                ]
            }
        ]
    })";

    WorldData world = DataLoader::loadFromStrings(goodsJson, citiesJson);

    CHECK(world.goods.size() == 1);
    CHECK(world.cities.size() == 1);
    CHECK(world.goods[0].id == "good_food");
    CHECK(world.goods[0].basePricePerTon == 650);
    CHECK(world.cities[0].id == "city_test");
    CHECK(world.cities[0].coordX == doctest::Approx(5.0));
}