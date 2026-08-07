#pragma once
#include <string>
#include <vector>
#include <kotr/core/Types.hpp>
#include <kotr/core/EventBus.hpp>
#include <kotr/core/TimeSystem.hpp>
#include <kotr/economy/EconomySystem.hpp>

namespace kotr::contracts {

    enum class ContractType {
        Normal, Urgent, Dangerous, Illegal, Story, Faction, Special
    };

    // Кто перевозит (единый агент по исправленной модели)
    enum class CarrierKind { Player, Named, MassAI };

    /// Перевозчик, участвующий в контракте.
    /// employerId — кто получает выручку: "player", id именного-нанимателя
    /// или собственный id (самозанятый / массовка).
    struct CarrierRef {
        std::string id;                 // "player", "drv_...", "ai_..."
        CarrierKind kind = CarrierKind::MassAI;
        std::string employerId;
        double speedKmh = 50.0;         // в 7b-3 придёт из VehicleSystem
        double capacityKg = 0.0;
    };

    /// Участник контракта (рейс в пути)
    struct ContractParticipant {
        CarrierRef carrier;
        double massTons = 0.0;
        core::GameMinutes joinTime = 0;
        core::GameMinutes arrivalTime = 0;
        core::GameMinutes deadline = 0;
        bool delivered = false;
    };

    /// Контракт = общий пул спроса (исправленная модель).
    /// Висит на доске, пока спрос не покрыт; брать могут многие — все они конкуренты.
    struct ContractOffer {
        std::string id;
        core::CityId origin;
        core::CityId destination;
        core::GoodId good;
        double demandTons = 0.0;        // сколько нужно отгрузить
        double deliveredTons = 0.0;     // сколько отгружено
        double distanceKm = 0.0;
        ContractType type = ContractType::Normal;
        bool active = true;             // пока спрос не покрыт
        core::GameMinutes createdAt = 0;

        bool firstDelivered = false;
        std::string firstDelivererId;
        bool firstDelivererEligible = false; // лицензия: только Player/Named

        std::vector<ContractParticipant> participants;
    };

    // === События ===

    struct ContractGeneratedEvent { ContractOffer offer; };

    struct ContractJoinedEvent {
        std::string contractId;
        std::string carrierId;
        CarrierKind kind;
        double massTons;
    };

    struct ContractDeliveryArrivedEvent {
        std::string contractId;
        std::string carrierId;
        CarrierKind kind;
        std::string employerId;
        double massTons;
        double distanceKm;
        core::Money payout;
        bool onTime;
        std::string good;
    };

    struct ContractFirstDeliveryEvent {
        std::string contractId;
        std::string carrierId;
        CarrierKind kind;
        bool licenseEligible;
    };

    struct ContractDeactivatedEvent { std::string contractId; };

    /// Система контрактов (исправленная модель конкуренции за общий спрос).
    class ContractSystem {
    public:
        static constexpr double AVERAGE_SPEED_KMH = 50.0;
        static constexpr int MAX_ACTIVE_CONTRACTS = 20;
        static constexpr double MAX_CONTRACT_TONS = 60.0;
        static constexpr double MIN_CONTRACT_TONS = 5.0;

        ContractSystem(core::EventBus& eventBus,
            economy::EconomySystem& economy,
            core::TimeSystem& time);

        // --- Симуляция (генерация + разрешение доставок) ---
        void update(core::GameTime now);
        void generateContracts(core::GameTime now);
        void resolveDeliveries(core::GameTime now);

        // --- Присоединение к контракту (взятие НЕ снимает с доски) ---
        bool joinContract(const std::string& contractId, const CarrierRef& carrier,
            double massTons, core::GameMinutes startDelayMinutes = 0);

        // --- Запросы ---
        [[nodiscard]] std::vector<ContractOffer> getAvailableContracts() const; // только активные
        [[nodiscard]] const ContractOffer* getContract(const std::string& id) const;
        [[nodiscard]] double remainingTons(const ContractOffer& c) const;
        [[nodiscard]] double inTransitTons(const ContractOffer& c) const;

    private:
        ContractOffer* findContract(const std::string& id);
        [[nodiscard]] double calcDistance(const core::CityId& from, const core::CityId& to) const;
        [[nodiscard]] double calcRouteRisk(const core::CityId& from, const core::CityId& to) const;
        [[nodiscard]] core::Money calcPayout(const ContractOffer& c, double massTons, core::GameTime now) const;
        [[nodiscard]] static double bufferFor(ContractType type);
        [[nodiscard]] static double urgencyFactorFor(ContractType type);

        core::EventBus& eventBus_;
        economy::EconomySystem& economy_;
        core::TimeSystem& time_;
        std::vector<ContractOffer> contracts_;
        int nextContractId_ = 1;
    };

} // namespace kotr::contracts