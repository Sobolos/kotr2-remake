#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "kotr/core/EventBus.hpp"
#include "kotr/finance/FinanceSystem.hpp"
#include "kotr/market/MarketSystem.hpp"
#include "kotr/contracts/ContractSystem.hpp"
#include "kotr/economy/EconomySystem.hpp"
#include "kotr/core/TimeSystem.hpp"

using namespace kotr::core;
using namespace kotr::finance;
using namespace kotr::market;

namespace contracts = kotr::contracts;

// === Тесты: FinanceSystem ===

TEST_CASE("FinanceSystem: initial balance and transactions") {
    EventBus bus;
    FinanceSystem finance(bus);

    SUBCASE("Начальный баланс") {
        finance.setInitialBalance(2500); // По Balance Prototype: 2500₽ старт
        CHECK(finance.balance() == 2500);
    }

    SUBCASE("Добавление дохода") {
        finance.setInitialBalance(2500);
        finance.addIncome(610, TransactionCategory::ContractPayout, "Ельнино → Шахты");
        CHECK(finance.balance() == 3110);
    }

    SUBCASE("Добавление расхода") {
        finance.setInitialBalance(2500);
        finance.addExpense(105, TransactionCategory::FuelCost, "Топливо");
        CHECK(finance.balance() == 2395);
    }

    SUBCASE("Банкротство при уходе в минус") {
        finance.setInitialBalance(100);
        bool bankruptcyTriggered = false;
        bus.subscribe<BankruptcyEvent>([&](const BankruptcyEvent& e) {
            bankruptcyTriggered = true;
            CHECK(e.debt < 0);
            });

        finance.addExpense(150, TransactionCategory::RepairCost, "Ремонт");
        CHECK(finance.balance() == -50);
        CHECK(finance.isBankrupt());
        CHECK(bankruptcyTriggered);
    }

    SUBCASE("canAfford проверяет баланс") {
        finance.setInitialBalance(2500);
        CHECK(finance.canAfford(2500));
        CHECK(finance.canAfford(100));
        CHECK_FALSE(finance.canAfford(3000));
    }
}

TEST_CASE("FinanceSystem: events and statistics") {
    EventBus bus;
    FinanceSystem finance(bus);

    SUBCASE("Событие изменения денег") {
        finance.setInitialBalance(1000);

        Money lastBalance = 0;
        Money lastDelta = 0;
        int eventCount = 0;

        bus.subscribe<MoneyChangedEvent>([&](const MoneyChangedEvent& e) {
            lastBalance = e.newBalance;
            lastDelta = e.delta;
            eventCount++;
            });

        finance.addIncome(500, TransactionCategory::ContractPayout);
        CHECK(eventCount == 1);
        CHECK(lastBalance == 1500);
        CHECK(lastDelta == 500);
    }

    SUBCASE("Статистика доходов и расходов") {
        finance.setInitialBalance(1000);
        finance.addIncome(500, TransactionCategory::ContractPayout);
        finance.addIncome(300, TransactionCategory::ContractPayout);
        finance.addExpense(200, TransactionCategory::FuelCost);
        finance.addExpense(100, TransactionCategory::RepairCost);

        CHECK(finance.totalIncome() == 800);
        CHECK(finance.totalExpenses() == 300);
        CHECK(finance.history().size() == 4); // 4 транзакции
    }
}

// === Тесты: MarketSystem ===

TEST_CASE("MarketSystem: market share calculation") {
    EventBus bus;
    MarketSystem market(bus);

    SUBCASE("Формула доли рынка") {
        // По DMS: MarketShare = PlayerCapacity / (Player + Competitors) * 100
        market.setPlayerCapacity(5000);      // 5 тонн (ЗиЛ)
        market.setCompetitorCapacity("comp_1", 10000); // 10 тонн
        market.setCompetitorCapacity("comp_2", 5000);  // 5 тонн

        // Total = 5000 + 10000 + 5000 = 20000
        // Share = 5000 / 20000 * 100 = 25%
        CHECK(market.getMarketShare() == doctest::Approx(25.0));
        CHECK(market.getTotalMarketCapacity() == doctest::Approx(20000.0));
    }

    SUBCASE("Пустой рынок") {
        CHECK(market.getMarketShare() == doctest::Approx(0.0));
        CHECK(market.getTotalMarketCapacity() == doctest::Approx(0.0));
    }

    SUBCASE("Только игрок на рынке") {
        market.setPlayerCapacity(5000);
        CHECK(market.getMarketShare() == doctest::Approx(100.0));
    }
}

TEST_CASE("MarketSystem: victory condition") {
    EventBus bus;
    MarketSystem market(bus);

    SUBCASE("Победа при >51%") {
        // По Balance Prototype: рынок ~1265 тонн, и он примерно постоянен.
        // Игрок растёт, ПОГЛОЩАЯ ёмкость конкурентов (найм с тягачами),
        // поэтому сумма "игрок + конкуренты" остаётся ~прежней.
        market.setCompetitorCapacity("mass_ai", 350000);      // было 750т, часть нанята
        market.setCompetitorCapacity("named_drivers", 270000); // было 510т, часть нанята

        // Игрок с 5 тоннами (ЗиЛ) — меньше 1%
        market.setPlayerCapacity(5000);
        CHECK(market.getMarketShare() < 1.0);
        CHECK_FALSE(market.isVictoryReached());

        // Игрок поглотил ёмкость: 650т против 620т у конкурентов
        market.setPlayerCapacity(650000);
        // Total = 650000 + 620000 = 1270000 → share = 650/1270 ≈ 51.2%
        CHECK(market.getMarketShare() > 51.0);
        CHECK(market.isVictoryReached());
    }

    SUBCASE("Событие победы") {
        market.setCompetitorCapacity("comp_1", 10000);

        bool victoryTriggered = false;
        double finalShare = 0.0;
        bus.subscribe<VictoryConditionReachedEvent>([&](const VictoryConditionReachedEvent& e) {
            victoryTriggered = true;
            finalShare = e.finalSharePercent;
            });

        // Переход через 51%
        market.setPlayerCapacity(9000); // 9000/(9000+10000) = 47.4% — ещё нет
        CHECK_FALSE(victoryTriggered);

        market.setPlayerCapacity(11000); // 11000/(11000+10000) = 52.4% — победа!
        CHECK(victoryTriggered);
        CHECK(finalShare > 51.0);
    }
}

TEST_CASE("MarketSystem: competitor management") {
    EventBus bus;
    MarketSystem market(bus);

    SUBCASE("Удаление конкурента увеличивает долю") {
        market.setPlayerCapacity(5000);
        market.setCompetitorCapacity("comp_1", 5000);
        market.setCompetitorCapacity("comp_2", 5000);

        double shareBefore = market.getMarketShare();
        CHECK(shareBefore == doctest::Approx(33.33).epsilon(0.01));

        // Удаляем одного конкурента
        market.removeCompetitor("comp_1");
        double shareAfter = market.getMarketShare();
        CHECK(shareAfter == doctest::Approx(50.0));
        CHECK(market.competitorCount() == 1);
    }

    SUBCASE("Событие изменения доли рынка") {
        market.setPlayerCapacity(5000);
        market.setCompetitorCapacity("comp_1", 5000);

        int eventCount = 0;
        double lastShare = 0.0;
        bus.subscribe<MarketShareChangedEvent>([&](const MarketShareChangedEvent& e) {
            eventCount++;
            lastShare = e.newSharePercent;
            });

        market.setCompetitorCapacity("comp_2", 10000);
        CHECK(eventCount > 0);
        CHECK(lastShare == doctest::Approx(25.0));
    }
}

// === Тесты: Интеграция контрактов с финансами ===

TEST_CASE("Integration: contract completion adds income") {
    EventBus bus;
    FinanceSystem finance(bus);
    finance.setInitialBalance(2500);

    // Подписываемся на доставку и добавляем доход (event-driven по TAD)
    bus.subscribe<contracts::ContractDeliveryArrivedEvent>(
        [&](const contracts::ContractDeliveryArrivedEvent& e) {
            finance.addIncome(e.payout, TransactionCategory::ContractPayout,
                "Contract " + e.contractId);
        });

    // Публикуем событие доставки (один раз)
    contracts::ContractDeliveryArrivedEvent event;
    event.contractId = "ctr_0001";
    event.payout = 958;
    event.onTime = true;
    bus.publish(event);

    CHECK(finance.balance() == 2500 + 958);
    CHECK(finance.totalIncome() == 958);
}

// === Тесты: Сценарий из Balance Prototype ===

TEST_CASE("Integration: early game survival scenario") {
    EventBus bus;
    FinanceSystem finance(bus);
    MarketSystem market(bus);

    // По Balance Prototype: Старт — 2500₽, 1 ЗиЛ (5 тонн), рынок ~1265 тонн
    finance.setInitialBalance(2500);
    market.setPlayerCapacity(5000); // ЗиЛ: 5 тонн
    market.setCompetitorCapacity("mass_ai", 750000);
    market.setCompetitorCapacity("named_drivers", 510000);

    SUBCASE("Стартовая доля рынка <1%") {
        CHECK(market.getMarketShare() < 1.0);
    }

    SUBCASE("Рейс Южный ↔ Ельнино приносит ~572₽ чистыми") {
        Money startBalance = finance.balance();

        // Доход: 5т × 30км × 4.5₽ × 1.1 = 742₽
        finance.addIncome(742, TransactionCategory::ContractPayout, "Продукты");

        // Расходы: топливо 105 + износ 45 + мелочи 20 = 170₽
        finance.addExpense(105, TransactionCategory::FuelCost);
        finance.addExpense(45, TransactionCategory::RepairCost);
        finance.addExpense(20, TransactionCategory::Other);

        Money netProfit = finance.balance() - startBalance;
        CHECK(netProfit == 742 - 105 - 45 - 20); // = 572₽
    }

    SUBCASE("За 3 дня можно накопить на ремонт ЗиЛа") {
        // 2 рейса в день × 572₽ = 1144₽/день
        for (int day = 0; day < 3; ++day) {
            for (int trip = 0; trip < 2; ++trip) {
                finance.addIncome(742, TransactionCategory::ContractPayout);
                finance.addExpense(170, TransactionCategory::FuelCost);
            }
        }

        // За 3 дня: 6 рейсов × 572₽ = 3432₽
        // Стартовый капитал 2500 + 3432 = 5932₽
        CHECK(finance.balance() > 5900);
        CHECK(finance.canAfford(1500)); // Хватает на ремонт ЗиЛа
    }
}