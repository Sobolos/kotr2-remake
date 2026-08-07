#include "kotr/vehicles/VehicleSystem.hpp"
#include <algorithm>

namespace kotr::vehicles {

    void VehicleSystem::registerModel(const VehicleModelData& model) {
        models_[model.id] = model;
    }

    std::string VehicleSystem::createInstance(const std::string& modelId,
        const std::string& ownerId, double condition) {
        VehicleInstance inst;
        inst.id = "veh_inst_" + std::to_string(nextInstanceId_++);
        inst.modelRef = modelId;
        inst.ownerId = ownerId;
        inst.condition = condition;
        instances_[inst.id] = inst;
        return inst.id;
    }

    const VehicleModelData* VehicleSystem::getModel(const std::string& modelRef) const {
        auto it = models_.find(modelRef);
        return it != models_.end() ? &it->second : nullptr;
    }

    const VehicleInstance* VehicleSystem::getInstance(const std::string& instanceId) const {
        auto it = instances_.find(instanceId);
        return it != instances_.end() ? &it->second : nullptr;
    }

    VehicleInstance* VehicleSystem::findInstance(const std::string& instanceId) {
        auto it = instances_.find(instanceId);
        return it != instances_.end() ? &it->second : nullptr;
    }

    double VehicleSystem::calcEffectiveSpeedKmh(const std::string& instanceId,
        double cargoKg, double roadLimitKmh) const {
        const auto* inst = getInstance(instanceId);
        if (!inst) return 0.0;
        const auto* model = getModel(inst->modelRef);
        if (!model) return 0.0;

        // Удельная мощность падает с грузом (по Vehicle Design §9.3)
        double totalTons = (model->massKg + cargoKg) / 1000.0;
        double pw = model->enginePowerKw / std::max(0.1, totalTons);
        double pwFactor = std::clamp(pw / PW_REFERENCE, PW_MIN_FACTOR, 1.0);

        // Битая машина едет медленнее
        double condFactor = 0.7 + 0.3 * (inst->condition / 100.0);

        double speed = model->topSpeedKmh * pwFactor * condFactor * inst->speedModifier;
        return std::min(speed, roadLimitKmh);
    }

    double VehicleSystem::getMarketCapacityKg(const std::string& instanceId) const {
        const auto* inst = getInstance(instanceId);
        if (!inst) return 0.0;
        const auto* model = getModel(inst->modelRef);
        return model ? model->marketPayloadKg : 0.0;
    }

    void VehicleSystem::setCargoLoad(const std::string& instanceId, double cargoKg) {
        auto* inst = findInstance(instanceId);
        if (inst) inst->cargoLoadKg = cargoKg;
    }

    void VehicleSystem::setCondition(const std::string& instanceId, double condition) {
        auto* inst = findInstance(instanceId);
        if (inst) inst->condition = std::clamp(condition, 0.0, 100.0);
    }

    void VehicleSystem::applyUpgrade(const std::string& instanceId, const std::string& upgradeTag) {
        auto* inst = findInstance(instanceId);
        if (!inst) return;

        // Стартовая таблица апгрейдов (позже — в JSON, по TAD data-driven)
        if (upgradeTag == "Upgrade.Engine.Turbo") {
            inst->speedModifier *= 1.08;
        }
        else if (upgradeTag == "Upgrade.Engine.Chip") {
            inst->speedModifier *= 1.05;
        }
        else if (upgradeTag == "Upgrade.Maintenance.Pro") {
            inst->reliabilityModifier *= 1.15;
        }
    }

} // namespace kotr::vehicles