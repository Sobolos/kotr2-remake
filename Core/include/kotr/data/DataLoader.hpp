#pragma once
#include <string>
#include <vector>
#include <kotr/data/GoodData.hpp>
#include <kotr/data/CityData.hpp>

namespace kotr::data {

    /// Результат загрузки данных.
    struct WorldData {
        std::vector<GoodData> goods;
        std::vector<CityData> cities;
        std::vector<CityGoodState> cityGoods;  // Начальное состояние товаров в городах

        [[nodiscard]] bool isValid() const {
            return !goods.empty() && !cities.empty();
        }
    };

    /// Загрузчик данных из JSON (data-driven по TAD).
    /// Поддерживает загрузку из файлов и из строк (для тестов).
    class DataLoader {
    public:
        /// Загрузить мир из JSON-строк (для unit-тестов).
        static WorldData loadFromStrings(const std::string& goodsJson,
            const std::string& citiesJson);

        /// Загрузить мир из файлов.
        static WorldData loadFromFiles(const std::string& goodsPath,
            const std::string& citiesPath);

    private:
        static GoodData parseGood(const void* jsonObj);  // nlohmann::json
        static CityData parseCity(const void* jsonObj);
        static CityGoodState parseCityGood(const void* jsonObj);
    };

} // namespace kotr::data