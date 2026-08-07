#include "kotr/finance/FinanceSystem.hpp"

namespace kotr::finance {

    using namespace kotr::core;

    FinanceSystem::FinanceSystem(EventBus& eventBus)
        : eventBus_(eventBus) {}

    void FinanceSystem::setInitialBalance(Money amount) {
        balance_ = amount;
    }

    void FinanceSystem::addIncome(Money amount, TransactionCategory category,
        const std::string& description) {
        applyTransaction(amount, category, description);
    }

    void FinanceSystem::addExpense(Money amount, TransactionCategory category,
        const std::string& description) {
        applyTransaction(-amount, category, description);
    }

    void FinanceSystem::applyTransaction(Money amount, TransactionCategory category,
        const std::string& description) {
        Money oldBalance = balance_;
        balance_ += amount;

        history_.push_back({ amount, category, description });
        eventBus_.publish(MoneyChangedEvent{ balance_, amount });

        // Проверяем банкротство при переходе через 0
        if (oldBalance >= 0 && balance_ < 0) {
            eventBus_.publish(BankruptcyEvent{ balance_ });
        }
    }

    Money FinanceSystem::totalIncome() const {
        Money total = 0;
        for (const auto& t : history_) {
            if (t.amount > 0) total += t.amount;
        }
        return total;
    }

    Money FinanceSystem::totalExpenses() const {
        Money total = 0;
        for (const auto& t : history_) {
            if (t.amount < 0) total += (-t.amount);
        }
        return total;
    }

} // namespace kotr::finance