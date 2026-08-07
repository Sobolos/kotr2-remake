#include "kotr/data/DataLoader.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace kotr::data {

    using json = nlohmann::json;
    using namespace kotr::core;

    // === Вспомогательные парсеры ===

    static GoodCategory parseGoodCategory(const std::string& s) {
        if (s == "raw") return GoodCategory::Raw;
        if (s == "industrial") return GoodCategory::Industrial;
        if (s == "consumer") return GoodCategory::Consumer;
        if (s == "fuel") return GoodCategory::Fuel;
        if (s == "parts") return GoodCategory::Parts;
        if (s == "special") return GoodCategory::Special;
        if (s == "illegal") return GoodCategory::Illegal;
        return GoodCategory::Consumer;
    }

    static StorageClass parseStorageClass(const std::string& s) {
        if (s == "bulk") return StorageClass::Bulk;
        if (s == "liquid") return StorageClass::Liquid;
        if (s == "perishable") return StorageClass::Perishable;
        if (s == "valuable") return StorageClass::Valuable;
        if (s == "dangerous") return StorageClass::Dangerous;
        return StorageClass::Standard;
    }

    static CitySpecialization parseCitySpecialization(const std::string& s) {
        if (s == "logistics") return CitySpecialization::Logistics;
        if (s == "mining") return CitySpecialization::Mining;
        if (s == "forest") return CitySpecialization::Forest;
        if (s == "oil") return CitySpecialization::Oil;
        if (s == "agriculture") return CitySpecialization::Agriculture;
        if (s == "metal") return CitySpecialization::Metal;
        if (s == "port") return CitySpecialization::Port;
        if (s == "mountain") return CitySpecialization::Mountain;
        if (s == "transit") return CitySpecialization::Transit;
        return CitySpecialization::Logistics;
    }

    // === Парсинг GoodData ===

    GoodData DataLoader::parseGood(const void* jsonObj) {
        const auto& j = *static_cast<const json*>(jsonObj);
        GoodData good;

        good.id = j.at("id").get<std::string>();
        good.nameKey = j.value("name_key", "");
        good.category = parseGoodCategory(j.value("category", "consumer"));
        good.storageClass = parseStorageClass(j.value("storage_class", "standard"));
        good.basePricePerTon = j.value("base_price_per_ton", static_cast<int64_t>(0));
        good.cargoValuePerTon = j.value("cargo_value_per_ton", static_cast<int64_t>(0));
        good.baseTariffPerTonKm = j.value("base_tariff_per_ton_km", static_cast<int64_t>(0));
        good.volatility = j.value("volatility", 0.0);
        good.dangerModifier = j.value("danger_modifier", 0.0);
        good.legal = j.value("legal", true);
        good.perishable = j.value("perishable", false);

        if (j.contains("tags")) {
            for (const auto& tag : j["tags"]) {
                good.tags.push_back(tag.get<std::string>());
            }
        }

        return good;
    }

    // === Парсинг CityData ===

    CityData DataLoader::parseCity(const void* jsonObj) {
        const auto& j = *static_cast<const json*>(jsonObj);
        CityData city;

        city.id = j.at("id").get<std::string>();
        city.nameKey = j.value("name_key", "");
        city.specialization = parseCitySpecialization(j.value("specialization", "logistics"));
        city.economicPower = j.value("economic_power", 1.0);
        city.storageCapacityMultiplier = j.value("storage_capacity_multiplier", 1.0);
        city.fuelBasePrice = j.value("fuel_base_price", static_cast<int64_t>(10));
        city.partsBaseAvailability = j.value("parts_base_availability", 0.7);
        city.baseDanger = j.value("base_danger", 0.3);
        city.coordX = j.value("coord_x", 0.0);
        city.coordY = j.value("coord_y", 0.0);

        if (j.contains("tags")) {
            for (const auto& tag : j["tags"]) {
                city.tags.push_back(tag.get<std::string>());
            }
        }

        return city;
    }

    // === Парсинг CityGoodState ===

    CityGoodState DataLoader::parseCityGood(const void* jsonObj) {
        const auto& j = *static_cast<const json*>(jsonObj);
        CityGoodState state;

        state.goodId = j.at("good_id").get<std::string>();
        state.supply = j.value("initial_supply", 0.0);
        state.demand = j.value("initial_demand", 0.0);
        state.productionPerDay = j.value("production_per_day", 0.0);
        state.consumptionPerDay = j.value("consumption_per_day", 0.0);
        state.currentPrice = 0;  // Рассчитывается при инициализации

        return state;
    }

    // === Загрузка из строк ===

    WorldData DataLoader::loadFromStrings(const std::string& goodsJson,
        const std::string& citiesJson) {
        WorldData world;

        // Парсим товары
        auto goodsRoot = json::parse(goodsJson);
        for (const auto& item : goodsRoot.at("goods")) {
            world.goods.push_back(parseGood(&item));
        }

        // Парсим города и их товары
        auto citiesRoot = json::parse(citiesJson);
        for (const auto& cityItem : citiesRoot.at("cities")) {
            world.cities.push_back(parseCity(&cityItem));

            // Парсим товары города
            if (cityItem.contains("goods")) {
                CityId cityId = cityItem.at("id").get<std::string>();
                for (const auto& goodItem : cityItem["goods"]) {
                    CityGoodState state = parseCityGood(&goodItem);
                    world.cityGoods.push_back(state);
                }
            }
        }

        return world;
    }

    // === Загрузка из файлов ===

    WorldData DataLoader::loadFromFiles(const std::string& goodsPath,
        const std::string& citiesPath) {
        auto readFile = [](const std::string& path) -> std::string {
            std::ifstream file(path);
            if (!file.is_open()) {
                throw std::runtime_error("Cannot open file: " + path);
            }
            std::stringstream buffer;
            buffer << file.rdbuf();
            return buffer.str();
            };

        std::string goodsJson = readFile(goodsPath);
        std::string citiesJson = readFile(citiesPath);

        return loadFromStrings(goodsJson, citiesJson);
    }

} // namespace kotr::data