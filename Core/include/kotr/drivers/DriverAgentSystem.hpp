#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <random>
#include <kotr/core/Types.hpp>
#include <kotr/core/EventBus.hpp>
#include <kotr/core/TimeSystem.hpp>
#include <kotr/economy/EconomySystem.hpp>
#include <kotr/contracts/ContractSystem.hpp>
#include <kotr/drivers/DriverSystem.hpp>

namespace kotr::vehicles { class VehicleSystem; }

namespace kotr::drivers {

    enum class AgentKind { Named, MassAI };

    /// Единый агент: нанятый / свободный именной / массовка (исправленная модель).
    struct DriverAgent {
        std::string id;
        AgentKind kind = AgentKind::MassAI;
        std::string employerId = "self";   // "self" | "player" | id именного
        double capacityKg = 7500;
        double speedKmh = 50.0;
        double aggression = 0.5;
        core::CityId currentBase;
        int failedTakeAttempts = 0;        // для mercy rule
        bool busy = false;                 // в рейсе / перегоне
        bool active = true;
        double deliveryScore = 0;
        core::Money money = 0;             // бизнес-деньги именного
        std::string vehicleRef;
    };

    /// Система агентов-водителей.
    /// Географический поиск заказов (база + соседи), mercy rule на 3-й базе,
    /// найм именных именованными, маршрутизация выручки работодателю.
    class DriverAgentSystem {
    public:
        static constexpr double NEIGHBOR_RADIUS_KM = 6.0;   // по прямой
        static constexpr double MIN_TAKE_INTEREST = 0.15;
        static constexpr double BASE_TAKE_RATE_PER_HOUR = 0.08;
        static constexpr int MERCY_VISITS = 2;              // на 3-й базе — гарантированно

        DriverAgentSystem(core::EventBus& eventBus,
            economy::EconomySystem& econ,
            contracts::ContractSystem& contractSys,
            DriverSystem& driverSys,
            core::TimeSystem& time);

        kotr::vehicles::VehicleSystem* vehicles_ = nullptr;

        void addMassAI(const std::string& id, double capacityKg, double aggression,
            core::CityId home);
        void addNamedAgent(const std::string& driverId, double capacityKg,
            double aggression, core::CityId home);

        void updateHour(core::GameTime now);

        [[nodiscard]] const DriverAgent* getAgent(const std::string& id) const;
        [[nodiscard]] size_t agentCount() const { return agents_.size(); }
        [[nodiscard]] double getCompetitorCapacityKg() const;
        [[nodiscard]] double getDeliveryScore(const std::string& id) const;

        void setSeed(uint64_t s) { rng_.seed(static_cast<std::mt19937::result_type>(s)); }
        void setBaseTakeRate(double rate) { baseTakeRate_ = rate; }

        void setVehicleSystem(kotr::vehicles::VehicleSystem* vs) { vehicles_ = vs; }
        void assignVehicle(const std::string& agentId, const std::string& vehicleInstanceId);

    private:
        [[nodiscard]] double straightDistanceKm(const core::CityId& a, const core::CityId& b) const;
        [[nodiscard]] bool isNeighbor(const core::CityId& a, const core::CityId& b) const;
        [[nodiscard]] double travelMinutes(const core::CityId& a, const core::CityId& b, double speed) const;
        void tryTakeFor(DriverAgent& a, core::GameTime now);
        void moveToBestNeighbor(DriverAgent& a);
        void paySalaries();

        core::EventBus& eventBus_;
        economy::EconomySystem& economy_;
        contracts::ContractSystem& contracts_;
        DriverSystem& drivers_;
        core::TimeSystem& time_;
        std::unordered_map<std::string, DriverAgent> agents_;
        std::mt19937 rng_{ 777 };
        double baseTakeRate_ = BASE_TAKE_RATE_PER_HOUR;
    };

} // namespace kotr::drivers