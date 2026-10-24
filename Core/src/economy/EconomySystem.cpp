#include "kotr/economy/EconomySystem.hpp"
#include <algorithm>
#include <cmath>

namespace kotr::economy {

    using namespace kotr::core;
    using namespace kotr::data;

    EconomySystem::EconomySystem(EventBus& eventBus, EconomyMode mode)
        : eventBus_(eventBus), mode_(mode) {}

    void EconomySystem::registerGood(const GoodData& good) {
        goods_[good.id] = good;
    }

    void EconomySystem::registerCity(const CityData& city) {
        cityData_[city.id] = city;
        CityState state;
        state.cityId = city.id;
        state.fuelPrice = city.fuelBasePrice;
        state.partsAvailability = city.partsBaseAvailability;
        cityStates_[city.id] = std::move(state);
    }

    void EconomySystem::initCityGood(const CityId& cityId, const CityGoodState& goodState) {
        auto it = cityStates_.find(cityId);
        if (it == cityStates_.end()) return;

        CityGoodState state = goodState;

        // Рассчитываем стартовую цену на основе начальных supply/demand,
        // чтобы цена была валидной сразу после инициализации (не 0).
        auto goodIt = goods_.find(goodState.goodId);
        if (goodIt != goods_.end()) {
            double deficitRatio = calcDeficitRatio(state.supply, state.demand);
            state.currentPrice = calcCurrentPrice(goodIt->second, deficitRatio, it->second);
        }

        it->second.goods[goodState.goodId] = state;
    }

    void EconomySystem::updateHour() {
        updateProductionConsumption();
        updatePrices();
    }

    void EconomySystem::updateProductionConsumption() {
        for (auto& [cityId, state] : cityStates_) {
            for (auto& [goodId, gs] : state.goods) {
                // supply += production_per_hour - spoiled
                double productionPerHour = gs.productionPerDay / 24.0;
                gs.supply += productionPerHour;

                // demand += consumption_per_hour
                double consumptionPerHour = gs.consumptionPerDay / 24.0;
                gs.demand += consumptionPerHour;

                // В Classic режиме supply не падает ниже минимума
                if (mode_ == EconomyMode::Classic) {
                    gs.supply = std::max(gs.supply, gs.demand * 0.5);
                }
            }
        }
    }

    void EconomySystem::updatePrices() {
        for (auto& [cityId, state] : cityStates_) {
            for (auto& [goodId, gs] : state.goods) {
                auto goodIt = goods_.find(goodId);
                if (goodIt == goods_.end()) continue;

                Money oldPrice = gs.currentPrice;
                double deficitRatio = calcDeficitRatio(gs.supply, gs.demand);
                Money newPrice = calcCurrentPrice(goodIt->second, deficitRatio, state);

                if (newPrice != oldPrice) {
                    gs.currentPrice = newPrice;
                    eventBus_.publish(PriceChangedEvent{ cityId, goodId, oldPrice, newPrice });
                }

                // Stockout event
                if (gs.supply <= 0.0 && gs.demand > 0.0) {
                    eventBus_.publish(StockoutEvent{ cityId, goodId });
                }
            }
        }
    }

    double EconomySystem::getDeficitRatio(const CityId& cityId, const GoodId& goodId) const {
        auto cityIt = cityStates_.find(cityId);
        if (cityIt == cityStates_.end()) return 1.0;

        auto goodIt = cityIt->second.goods.find(goodId);
        if (goodIt == cityIt->second.goods.end()) return 1.0;

        return calcDeficitRatio(goodIt->second.supply, goodIt->second.demand);
    }

    Money EconomySystem::getPrice(const CityId& cityId, const GoodId& goodId) const {
        auto cityIt = cityStates_.find(cityId);
        if (cityIt == cityStates_.end()) return 0;

        auto goodIt = cityIt->second.goods.find(goodId);
        if (goodIt == cityIt->second.goods.end()) return 0;

        return goodIt->second.currentPrice;
    }

    double EconomySystem::getSupply(const CityId& cityId, const GoodId& goodId) const {
        auto cityIt = cityStates_.find(cityId);
        if (cityIt == cityStates_.end()) return 0.0;

        auto goodIt = cityIt->second.goods.find(goodId);
        if (goodIt == cityIt->second.goods.end()) return 0.0;

        return goodIt->second.supply;
    }

    double EconomySystem::getDemand(const CityId& cityId, const GoodId& goodId) const {
        auto cityIt = cityStates_.find(cityId);
        if (cityIt == cityStates_.end()) return 0.0;

        auto goodIt = cityIt->second.goods.find(goodId);
        if (goodIt == cityIt->second.goods.end()) return 0.0;

        return goodIt->second.demand;
    }

    const CityState* EconomySystem::getCityState(const CityId& cityId) const {
        auto it = cityStates_.find(cityId);
        return it != cityStates_.end() ? &it->second : nullptr;
    }

    const GoodData* EconomySystem::getGoodData(const GoodId& goodId) const {
        auto it = goods_.find(goodId);
        return it != goods_.end() ? &it->second : nullptr;
    }

    void EconomySystem::deliverGood(const CityId& cityId, const GoodId& goodId, double tons) {
        auto cityIt = cityStates_.find(cityId);
        if (cityIt == cityStates_.end()) return;

        auto goodIt = cityIt->second.goods.find(goodId);
        if (goodIt == cityIt->second.goods.end()) return;

        // При доставке: supply увеличивается, demand уменьшается
        goodIt->second.supply += tons;
        goodIt->second.demand = std::max(0.0, goodIt->second.demand - tons);
    }

    void EconomySystem::consumeGood(const CityId& cityId, const GoodId& goodId, double tons) {
        auto cityIt = cityStates_.find(cityId);
        if (cityIt == cityStates_.end()) return;

        auto goodIt = cityIt->second.goods.find(goodId);
        if (goodIt == cityIt->second.goods.end()) return;

        goodIt->second.supply = std::max(0.0, goodIt->second.supply - tons);
    }

    // === Формулы из Economy System Design ===

    double EconomySystem::calcDeficitRatio(double supply, double demand) const {
        // deficit_ratio = demand / max(supply, 1)
        return demand / std::max(supply, 1.0);
    }

    double EconomySystem::calcPriceFactor(double deficitRatio) const {
        // price_factor = clamp(0.6 + 0.4 * deficit_ratio, 0.5, 3.0)
        double factor = 0.6 + 0.4 * deficitRatio;
        return std::clamp(factor, 0.5, 3.0);
    }

    Money EconomySystem::calcCurrentPrice(const GoodData& good, double deficitRatio, const CityState& city) const {
        double priceFactor = calcPriceFactor(deficitRatio);

        // current_price = base_price * price_factor * event_factor * crisis_factor
        double rawPrice = static_cast<double>(good.basePricePerTon)
            * priceFactor
            * city.eventFactor
            * city.crisisFactor;

        // Пределы: min_price = base_price * 0.4, max_price = base_price * 4.0
        // Лимиты применяются к финальной цене после всех множителей (event_factor, crisis_factor)
        double minPrice = static_cast<double>(good.basePricePerTon) * 0.4;
        double maxPrice = static_cast<double>(good.basePricePerTon) * 4.0;

        rawPrice = std::clamp(rawPrice, minPrice, maxPrice);

        return static_cast<Money>(std::round(rawPrice));
    }

    const CityData* EconomySystem::getCityData(const CityId& cityId) const {
        auto it = cityData_.find(cityId);
        return it != cityData_.end() ? &it->second : nullptr;
    }

    std::vector<CityId> EconomySystem::getAllCityIds() const {
        std::vector<CityId> result;
        result.reserve(cityData_.size());
        for (const auto& [id, data] : cityData_) result.push_back(id);
        std::sort(result.begin(), result.end());
        return result;
    }

    std::vector<GoodId> EconomySystem::getAllGoodIds() const {
        std::vector<GoodId> result;
        result.reserve(goods_.size());
        for (const auto& [id, data] : goods_) result.push_back(id);
        std::sort(result.begin(), result.end());
        return result;
    }

} // namespace kotr::economy