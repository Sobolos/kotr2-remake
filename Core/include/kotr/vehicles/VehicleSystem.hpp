#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <kotr/core/Types.hpp>

namespace kotr::vehicles {

    enum class VehicleClass { Light, Medium, Heavy, Special };

    /// Статичная модель (по DMS: VehicleModelData).
    /// Цифры гейм-тюнингованные под Vehicle Design §33 (ранний 60–75, средний 75–90, поздний 85–100+).
    struct VehicleModelData {
        std::string id;                     // "veh_model_zil130"
        std::string nameKey;
        VehicleClass vehicleClass = VehicleClass::Light;
        std::vector<std::string> originTags; // "Vehicle.Origin.Domestic"
        core::Money priceNew = 0;
        double marketPayloadKg = 5000;      // идёт в долю рынка (прицепы НЕ считаются)
        double massKg = 4300;
        double enginePowerKw = 90;
        double topSpeedKmh = 80;
        double reliability = 0.5;           // 0–1
        double fuelConsumptionBase = 0.35;  // л/км
        double repairCostMultiplier = 1.0;
    };

    /// Экземпляр (по DMS: VehicleInstance, упрощённо)
    struct VehicleInstance {
        std::string id;
        std::string modelRef;
        std::string ownerId;
        double condition = 100;     // 0–100
        double cargoLoadKg = 0;
        double speedModifier = 1.0;       // апгрейды
        double reliabilityModifier = 1.0;
    };

    /// VehicleSystem (light): performance envelope из Vehicle Design §10.
    class VehicleSystem {
    public:
        static constexpr double PW_REFERENCE = 12.0;      // кВт/т для полного фактора
        static constexpr double PW_MIN_FACTOR = 0.6;
        static constexpr double DEFAULT_ROAD_LIMIT_KMH = 90.0;

        void registerModel(const VehicleModelData& model);
        std::string createInstance(const std::string& modelId, const std::string& ownerId,
            double condition = 100);

        [[nodiscard]] const VehicleModelData* getModel(const std::string& modelRef) const;
        [[nodiscard]] const VehicleInstance* getInstance(const std::string& instanceId) const;

        /// speed = top_speed × pw_factor × condition_factor × upgrade_mult,
        /// ограничена лимитом дороги (по Vehicle Design §10.1)
        [[nodiscard]] double calcEffectiveSpeedKmh(const std::string& instanceId,
            double cargoKg,
            double roadLimitKmh = DEFAULT_ROAD_LIMIT_KMH) const;

        [[nodiscard]] double getMarketCapacityKg(const std::string& instanceId) const;

        void setCargoLoad(const std::string& instanceId, double cargoKg);
        void setCondition(const std::string& instanceId, double condition);
        void applyUpgrade(const std::string& instanceId, const std::string& upgradeTag);

    private:
        VehicleInstance* findInstance(const std::string& instanceId);
        std::unordered_map<std::string, VehicleModelData> models_;
        std::unordered_map<std::string, VehicleInstance> instances_;
        int nextInstanceId_ = 1;
    };

} // namespace kotr::vehicles