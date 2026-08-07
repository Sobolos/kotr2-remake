#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <kotr/core/Types.hpp>
#include <kotr/core/EventBus.hpp>
#include <kotr/core/TimeSystem.hpp>
#include <kotr/data/GoodData.hpp>
#include <kotr/data/CityData.hpp>

namespace kotr::economy {

    // Режим экономики (по DMS: EconomyMode)
    enum class EconomyMode {
        Living,   // Живая экономика: полная динамика
        Classic   // Классическая: упрощённая, топливо/запчасти доступны
    };

    // === События экономики ===

    struct PriceChangedEvent {
        kotr::core::CityId cityId;
        kotr::core::GoodId goodId;
        kotr::core::Money oldPrice;
        kotr::core::Money newPrice;
    };

    struct StockoutEvent {
        kotr::core::CityId cityId;
        kotr::core::GoodId goodId;
    };

    struct FuelCrisisEvent {
        kotr::core::CityId cityId;
        bool started; // true = начался, false = закончился
    };

    /// Экономическая система.
    /// Управляет supply/demand, ценами, дефицитом, топливом.
    class EconomySystem {
    public:
        explicit EconomySystem(kotr::core::EventBus& eventBus, EconomyMode mode = EconomyMode::Living);

        // --- Инициализация ---
        void registerGood(const kotr::data::GoodData& good);
        void registerCity(const kotr::data::CityData& city);
        void initCityGood(const kotr::core::CityId& cityId, const kotr::data::CityGoodState& goodState);

        // --- Обновление (вызывается каждый игровой час) ---
        void updateHour();

        // --- Запросы ---
        [[nodiscard]] double getDeficitRatio(const kotr::core::CityId& cityId, const kotr::core::GoodId& goodId) const;
        [[nodiscard]] kotr::core::Money getPrice(const kotr::core::CityId& cityId, const kotr::core::GoodId& goodId) const;
        [[nodiscard]] double getSupply(const kotr::core::CityId& cityId, const kotr::core::GoodId& goodId) const;
        [[nodiscard]] double getDemand(const kotr::core::CityId& cityId, const kotr::core::GoodId& goodId) const;
        [[nodiscard]] const kotr::data::CityState* getCityState(const kotr::core::CityId& cityId) const;
        [[nodiscard]] const kotr::data::GoodData* getGoodData(const kotr::core::GoodId& goodId) const;
        [[nodiscard]] const kotr::data::CityData* getCityData(const kotr::core::CityId& cityId) const;
        [[nodiscard]] std::vector<kotr::core::CityId> getAllCityIds() const;
        [[nodiscard]] std::vector<kotr::core::GoodId> getAllGoodIds() const;

        // --- Действия ---
        void deliverGood(const kotr::core::CityId& cityId, const kotr::core::GoodId& goodId, double tons);
        void consumeGood(const kotr::core::CityId& cityId, const kotr::core::GoodId& goodId, double tons);

        // --- Режим ---
        [[nodiscard]] EconomyMode mode() const { return mode_; }
        void setMode(EconomyMode mode) { mode_ = mode; }

    private:
        // Формулы из Economy System Design
        [[nodiscard]] double calcDeficitRatio(double supply, double demand) const;
        [[nodiscard]] double calcPriceFactor(double deficitRatio) const;
        [[nodiscard]] kotr::core::Money calcCurrentPrice(const kotr::data::GoodData& good, double deficitRatio, const kotr::data::CityState& city) const;
        void updatePrices();
        void updateProductionConsumption();

        kotr::core::EventBus& eventBus_;
        EconomyMode mode_;

        std::unordered_map<kotr::core::GoodId, kotr::data::GoodData> goods_;
        std::unordered_map<kotr::core::CityId, kotr::data::CityData> cityData_;
        std::unordered_map<kotr::core::CityId, kotr::data::CityState> cityStates_;
    };

} // namespace kotr::economy