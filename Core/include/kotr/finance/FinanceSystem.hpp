#pragma once
#include <string>
#include <vector>
#include <kotr/core/Types.hpp>
#include <kotr/core/EventBus.hpp>

namespace kotr::finance {

    // Категория транзакции (для аналитики и отладки баланса)
    enum class TransactionCategory {
        ContractPayout,    // Оплата контракта
        FuelCost,          // Топливо
        RepairCost,        // Ремонт
        DriverSalary,      // Зарплата водителя
        FineOrBribe,       // Штраф или взятка
        VehiclePurchase,   // Покупка техники
        VehicleSale,       // Продажа техники
        HireCost,          // Найм водителя
        FactionService,    // Услуги фракций
        Other
    };

    /// Запись в журнале транзакций (по DMS: FinanceLedgerEntry)
    struct Transaction {
        kotr::core::Money amount = 0;  // >0 доход, <0 расход
        TransactionCategory category = TransactionCategory::Other;
        std::string description;
    };

    // === События финансов ===

    struct MoneyChangedEvent {
        kotr::core::Money newBalance;
        kotr::core::Money delta;
    };

    struct BankruptcyEvent {
        kotr::core::Money debt;
    };

    /// Финансовая система игрока.
    /// Отслеживает баланс, доходы и расходы.
    class FinanceSystem {
    public:
        explicit FinanceSystem(kotr::core::EventBus& eventBus);

        // --- Управление балансом ---
        void setInitialBalance(kotr::core::Money amount);
        void addIncome(kotr::core::Money amount, TransactionCategory category,
            const std::string& description = "");
        void addExpense(kotr::core::Money amount, TransactionCategory category,
            const std::string& description = "");

        // --- Запросы ---
        [[nodiscard]] kotr::core::Money balance() const { return balance_; }
        [[nodiscard]] bool isBankrupt() const { return balance_ < 0; }
        [[nodiscard]] bool canAfford(kotr::core::Money amount) const { return balance_ >= amount; }
        [[nodiscard]] const std::vector<Transaction>& history() const { return history_; }

        // --- Статистика ---
        [[nodiscard]] kotr::core::Money totalIncome() const;
        [[nodiscard]] kotr::core::Money totalExpenses() const;

    private:
        void applyTransaction(kotr::core::Money amount, TransactionCategory category,
            const std::string& description);

        kotr::core::EventBus& eventBus_;
        kotr::core::Money balance_ = 0;
        std::vector<Transaction> history_;
    };

} // namespace kotr::finance