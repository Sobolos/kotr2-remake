#pragma once
#include <string>
#include <vector>
#include <kotr/core/Types.hpp>

namespace kotr::data {

    // Категория товара (по DMS: GoodCategory)
    enum class GoodCategory {
        Raw,        // Сырьё (лес, уголь, руда)
        Industrial, // Промышленные (металл, запчасти)
        Consumer,   // Потребительские (продукты, потребтовары)
        Fuel,       // Топливо
        Parts,      // Запчасти
        Special,    // Особые (алмазы)
        Illegal     // Нелегальные
    };

    // Класс хранения (по DMS: storage_class)
    enum class StorageClass {
        Bulk,       // Сыпучий
        Liquid,     // Жидкость
        Perishable, // Скоропортящийся
        Valuable,   // Ценный
        Dangerous,  // Опасный
        Standard    // Обычный
    };

    /// Статические данные о товаре. Не меняются в рантайме.
    struct GoodData {
        core::GoodId id;                  // "good_food"
        std::string nameKey;              // "loc.good.food.name"
        GoodCategory category = GoodCategory::Consumer;
        StorageClass storageClass = StorageClass::Standard;

        core::Money basePricePerTon = 0;      // Базовая цена за тонну
        core::Money cargoValuePerTon = 0;     // Стоимость тонны (для риска, краж)
        core::Money baseTariffPerTonKm = 0;   // Базовая ставка за тонно-км
        double volatility = 0.0;              // Волатильность цены (0.0–1.0)
        double dangerModifier = 0.0;          // Опасность перевозки (0.0–1.0)
        bool legal = true;                    // Легальность
        bool perishable = false;              // Скоропортящийся

        std::vector<std::string> tags;        // Теги: "Cargo.Coal", "Cargo.Illegal"
    };

} // namespace kotr::data