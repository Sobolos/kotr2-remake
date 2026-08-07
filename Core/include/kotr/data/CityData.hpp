#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <kotr/core/Types.hpp>
#include <kotr/data/GoodData.hpp>

namespace kotr::data {

    // Специализация города (по DMS: CitySpecialization)
    enum class CitySpecialization {
        Logistics,
        Mining,
        Forest,
        Oil,
        Agriculture,
        Metal,
        Port,
        Mountain,
        Transit
    };

    /// Статические данные о городе. Не меняются в рантайме.
    struct CityData {
        core::CityId id;                    // "city_yuzhny"
        std::string nameKey;                // "loc.city.yuzhny.name"
        CitySpecialization specialization = CitySpecialization::Logistics;

        double economicPower = 1.0;             // Экономический вес города
        double storageCapacityMultiplier = 1.0; // Ёмкость складов
        core::Money fuelBasePrice = 10;         // Базовая цена топлива
        double partsBaseAvailability = 0.7;     // Доступность запчастей (0.0–1.0)
        double baseDanger = 0.3;                // Базовая опасность (0.0–1.0)

        // Координаты (по Road Network Design)
        double coordX = 0.0;
        double coordY = 0.0;

        std::vector<std::string> tags;
    };

    /// Рантайм-состояние товара в городе (по DMS: CityGoodState)
    struct CityGoodState {
        core::GoodId goodId;
        double supply = 0.0;              // Доступный запас (тонны)
        double demand = 0.0;              // Накопленный спрос (тонны)
        double productionPerDay = 0.0;    // Производство в день (тонны)
        double consumptionPerDay = 0.0;   // Потребление в день (тонны)
        core::Money currentPrice = 0;     // Текущая цена за тонну
    };

    /// Рантайм-состояние города (по DMS: CityState)
    struct CityState {
        core::CityId cityId;
        std::unordered_map<core::GoodId, CityGoodState> goods;

        // Топливо
        double fuelAvailable = 100.0;     // Доступное топливо (нормировано)
        core::Money fuelPrice = 10;       // Текущая цена топлива
        bool fuelStationClosed = false;   // АЗС закрыта?

        // Запчасти
        double partsAvailability = 0.7;   // Доступность запчастей (0.0–1.0)

        // Модификаторы событий
        double eventFactor = 1.0;
        double crisisFactor = 1.0;
    };

} // namespace kotr::data