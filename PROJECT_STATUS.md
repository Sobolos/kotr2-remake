# PROJECT_STATUS.md — Дальнобойщики 2 Remake: Core Layer
Дата: 07.08.2026. Документ для продолжения работы в новом чате.
Дизайн-документы (Vision, HL-GDD, TAD, DMS, Economy, Risk, Driver, Vehicle, Faction, Radio, Narrative, Road Network, Balance Prototype, World Bible) приложены отдельно и являются источником требований. Этот документ — состояние РЕАЛИЗАЦИИ.

## 0. Окружение и сборка
- Windows, Visual Studio 2022 Community (workload "Desktop development with C++"), CMake, C++20, MSVC.
- Путь проекта: `C:\Users\germa\OneDrive\Desktop\Дальнобойщики 2` (кириллица/пробелы — работает).
- Зависимости (header-only, в `Core/third_party/`): `doctest/doctest.h`, `nlohmann/json.hpp`.
- Сборка: `cmake -B build -G "Visual Studio 17 2022" -A x64` → `cmake --build build --config Debug`.
- ВНИМАНИЕ: существуют ДВА build-каталога: ручной `build/` и VS Open-Folder `out/build/x64-Debug/`. Проверять, какой актуален, перед запуском тестов.
- Тест-экзекьютаблы: KotRTests, KotRTestEconomy, KotRTestDataLoader, KotRTestContracts (файл `tests/test_contract.cpp`, ед. число!), KotRTestMarket, KotRTestDrivers, KotRTestDriverAgents, KotRTestVehicles, KotRTestLeaderboard.
- Git: НЕ инициализирован. ПЕРЕД новым чатом: `git init; git add .; git commit` (+ приватный репозиторий на GitHub по желанию).

## 1. Архитектурные принципы (соблюдены во всех модулях)
- Core Layer — чистый C++20 без UE; модули общаются через EventBus (шаблонный, по type_index).
- Data-driven: данные в JSON (`Core/data/goods.json`, `cities.json`); ID snake_case с префиксами (`city_`, `good_`, `drv_`, `veh_model_`).
- Деньги = int64 целые рубли; время = игровые минуты (GameTime.totalMinutes); масштаб 1:4 (1 игровой час = 900 реальных секунд).
- Детерминизм: seed-контроль RNG (`setSeed`), `setBaseTakeRate(1.0)` в тестах агентов.
- Интеграция в UE (план, не реализовано): модуль `KotRCore` через `.Build.cs`, подключающий `Core/include` + `Core/src`; три режима синхронизации машин (игрок — физика UE авторитетна и шлёт прогресс в Core ~10 Гц; видимые AI — спаун из Core-снапшота с мягкой коррекцией ≤5–10%; оффскрин — только Core).

## 2. Реализованные модули (все тесты зелёные на момент коммита)

### kotr::core
- `Types.hpp`: Money, GameMinutes, Reputation, EntityId; строковые ID (CityId, GoodId, DriverId, VehicleModelId, FactionId, ContractId...).
- `EventBus`: subscribe<T>/publish<T>/unsubscribe, HandlerId.
- `TimeSystem`: tick(realSec), события GameHourElapsed/GameDayElapsed/GameWeekElapsed, pause/resume.

### kotr::economy (EconomySystem)
- GoodData/CityData/CityGoodState/CityState по DMS. Режимы Living/Classic.
- Формулы: deficit_ratio = demand/max(supply,1); price_factor = clamp(0.6+0.4*deficit, 0.5, 3); цена в [0.4×, 4×] базы.
- updateHour (production/consumption), deliverGood, getAllCityIds/getAllGoodIds/getCityData.

### kotr::data (DataLoader)
- loadFromStrings/loadFromFiles; 11 городов × 9 товаров = 99 записей в `Core/data/`.

### kotr::contracts (ContractSystem) — ИСПРАВЛЕННАЯ МОДЕЛЬ (см. §3)
- ContractOffer = пул спроса: demandTons, deliveredTons, participants[], active, firstDelivered/firstDelivererId/firstDelivererEligible.
- CarrierRef{id, kind(Player/Named/MassAI), employerId, speedKmh, capacityKg}.
- `joinContract(id, carrier, massTons, startDelayMinutes=0)` — взятие НЕ снимает с доски; валидация capacity и remaining (demand − delivered − inTransit).
- Генерация: uncovered = demand−supply ≥ 5т (кап 60т); origin = город с макс. supply; тип: deficit≥2.5→Urgent, danger≥0.5→Dangerous, !legal→Illegal.
- Оплата В МОМЕНТ ДОСТАВКИ: tariff×mass×distance×demand_price_factor(clamp(0.7+0.3*deficit,0.7,2))×urgency×risk(1+0.35*routeRisk+0.25*cargoDanger)×illegal×competition_factor; опоздание ×0.8.
- competition_factor: 1 участник→1.15; >3→max(0.85, 1−0.05*(n−3)).
- Деактивация при deliveredTons ≥ demandTons; везущие доезжают.
- Дедлайн: join + (delay+travel)×buffer (Normal 1.6/Urgent 1.1/Dangerous 2.0/Illegal 1.8).
- События: ContractGenerated, ContractJoined, ContractDeliveryArrived{contractId, carrierId, kind, employerId, massTons, distanceKm, payout, onTime}, ContractFirstDelivery{licenseEligible = kind!=MassAI}, ContractDeactivated.

### kotr::finance / kotr::market
- FinanceSystem: баланс, транзакции (TransactionCategory), totalIncome/Expenses, BankruptcyEvent; setInitialBalance НЕ пишет в историю.
- MarketSystem: share = player/(player+competitors)×100; порог 51%; adjustPlayerCapacity(delta); события MarketShareChanged/VictoryConditionReached.

### kotr::drivers (DriverSystem)
- Persona/State по DMS; статусы FreeParked/FreeIndependent/HiredByPlayer/HiredByNamed/OnContract/Resting/OutOfAction/...
- Рынок труда: 10 видимых слотов, ротация; `hireDriver(id, employerId="player")` — только с парковки + лицензии; cost = signing_bonus + vehicle_buyout.
- Политики оплаты Fixed/Percent/FixedPlusBonus/BonusOnly (calcDailySalary).
- updateDay: fatigue+10, дрейф morale/loyalty, DriverSalaryDueEvent{driverId, employerId, amount}; обрабатывает HiredByPlayer И HiredByNamed И OnContract.
- Уничтожение машины → OutOfAction 1 день, employerRef="", возврат с Ersatz (50% ёмкости).
- calcBetrayalRisk = 0.35*LowLoyalty+0.15*Fatigue+0.15*Stress+0.15*PaymentDiscontent+0.10*HiddenTrait+0.10*CargoTemptation.
- `setLicenseSystem(LicenseSystem*)` — hasRequiredLicenses проверяет лицензии ИГРОКА ("player").

### kotr::drivers (DriverAgentSystem) — заменил CompetitorSystem (файлы CompetitorSystem УДАЛЕНЫ)
- DriverAgent{id, kind, employerId("self"/"player"/named), capacityKg, speedKmh, aggression, currentBase, failedTakeAttempts, busy, active, deliveryScore, money, vehicleRef}.
- География заказа: origin == currentBase ИЛИ isNeighbor (прямая ≤ NEIGHBOR_RADIUS_KM=6); перегон = задержка startDelay.
- Торги: interest = clamp((deficit−0.8)*0.5, 0, 1); шанс = interest×aggression×baseTakeRate; MIN_TAKE_INTEREST=0.15.
- MERCY RULE: failedTakeAttempts ≥ 2 → на 3-й базе гарантированное взятие; moveToBestNeighbor при неудаче.
- Доставка (по ContractDeliveryArrivedEvent): busy=false, currentBase=destination, score += mass*distance/100, разгрузка машины.
- Бизнес именных: DriverHiredEvent → работодатель−=totalCost; DriverSalaryDueEvent → именной-работодатель −=salary; выручка += payout по employerId. РАБОТОДАТЕЛЬ ПРОДОЛЖАЕТ ВОЗИТЬ САМ (решение пользователя).
- getCompetitorCapacityKg: массовка + самозанятые именные (свои + нанятые); нанятые игроком не входят.
- assignVehicle + setVehicleSystem(VehicleSystem*): ёмкость и скорость агента от тягача.

### kotr::vehicles (VehicleSystem, light)
- VehicleModelData{marketPayloadKg, massKg, enginePowerKw, topSpeedKmh, reliability,...}; VehicleInstance{condition, cargoLoadKg, speedModifier, reliabilityModifier}.
- calcEffectiveSpeedKmh(inst, cargoKg, roadLimit=90) = min(top × pwFactor × condFactor × speedMod, roadLimit); pwFactor = clamp((kw/totalTons)/12, 0.6, 1); condFactor = 0.7+0.3*cond/100.
- Тест-модели: zil130 (5т/4300кг/90кВт/80), kamaz5320 (10т/9000/210/90), scania113 (20т/12000/380/105) — попадают в диапазоны 60–75/75–90/85–100+.
- applyUpgrade: Upgrade.Engine.Turbo ×1.08, Upgrade.Engine.Chip ×1.05, Upgrade.Maintenance.Pro reliability ×1.15 (хардкод; позже — в JSON).

### kotr::leaderboard / kotr::licenses (ТЕКУЩАЯ версия — ПОДЛЕЖИТ УПРОЩЕНИЮ, см. §4)
- DeliveryLeaderboard: score = mass×distance×categoryMult×onTime(1.0/0.7); категории Food..Illegal + Overall; evaluatePeriod по GameWeekElapsed → LicenseAwardedEvent первому eligible (player/named).
- LicenseSystem: хранение множеств лицензий по owner; DriverSystem проверяет `required_licenses` (список строк) у игрока.
- ИЗВЕСТНЫЙ БАГ: в `DeliveryLeaderboard.hpp` нет `#include <random>` (ошибка std::mt19937) — ИСПРАВИТЬ ПЕРВЫМ ДЕЛОМ.

## 3. КАНОНИЧНАЯ модель конкуренции (согласована с пользователем; переписывает любые старые формулировки из ранних фаз)
1. Контракт = пул неудовлетворённого спроса; висит, пока спрос не покрыт; берут многие одновременно — все конкуренты.
2. Взятие не снимает с доски.
3. Деактивация по покрытию спроса; везущие доезжают и остаются конкурентами.
4. Первая доставка → лицензионный зачёт ТОЛЬКО player/named; массовка первая — никто не получает; остальные продолжают.
5. Заказы берутся с текущей базы или соседних; mercy rule на 3-й неудачной базе.
6. Именные нанимают именных: выкуп платит наниматель, выручка ему, зарплаты ежедневно; наниматель продолжает возить сам.
7. (Запланировано, не реализовано) Нанятый водитель с лицензией и капиталом может уволиться и стать работодателем (триггер: licenses≥1 && money≥порог && рядом кандидат; событие DriverBecameEmployer).

## 4. СОГЛАСОВАННЫЕ РЕШЕНИЯ, НЕ РЕАЛИЗОВАННЫЕ (очередь)
A. МАЛЫЙ ПАТЧ (сделать первым):
   1) `#include <random>` в DeliveryLeaderboard.hpp.
   2) Упрощение лицензий: УБРАТЬ категории; лицензия = факт допуска к найму; количество = число побед с учётом серии (2 подряд → 2 лицензии, 3+ подряд → 3); `required_licenses` у persona становится числом 0–3; hasRequiredLicenses сравнивает количества. Переписать DeliveryLeaderboard/LicenseSystem/DriverSystem + тесты.
B. VehicleSystem расширение (с фазой Risk): DamagePartState (12 частей по Vehicle Design §15), износ WearGain, BreakdownRisk, апгрейды в JSON.
C. RoadGraph (фаза 9): RoadNode/RoadEdge (length, road_class A–F, surface, base_danger, speed_limit, faction_zones), RiskPocket/RepairPoint на рёбрах; позиция = (edge_id, progress); сопоставление с UE-сплайнами по edge_id.
D. Balance Visualizer (фаза 10): DebugServer (HTTP/WebSocket) в Core + веб-фронт (Canvas): карта городов, точки машин (красная/синие/зелёные), клик по городу → supply/demand, активные контракты и участники гонки, наймы, лицензии. Автопилот-плейтест без UE.
E. SaveSystem (фаза 11): сериализация всех состояний, версионирование.

## 5. Роадмап
- Фаза 8: RiskSystem light — PoliceHeat (0–100, быстрый распад), BanditPressure, базовый ThreatDirector (ThreatMeter, пороги 15/35/60/90), правила милосердия; интеграция с ContractSystem (route_risk) и RadioSystem-заглушкой.
- Фаза 9: RoadGraph + перевод ContractSystem/агентов на граф.
- Фаза 10: Visualizer. Фаза 11: SaveSystem. Далее FactionSystem, RadioSystem, Narrative.

## 6. Баланс-константы (из Balance Prototype, стартовые)
Старт 2500₽; ЗиЛ 5т, состояние 40%; топливо 10₽/л; зарплата-оклад 150₽/день; выкуп КамАЗа ~25000₽ (+signing 500); рынок ~1265т (массовка 750т + именные 510т + игрок 5т); цель 51% = 645т ≈ найм ~51 водителя (~40 млн ₽); ранний рейс ~572₽ чистыми.

## 7. Уроки/грабли (не повторять!)
- `using namespace X;` НЕ вводит короткое имя `X::` — нужен `namespace contracts = kotr::contracts;`.
- Пропущенный `#include` даёт каскад странных ошибок (первая ошибка — корневая).
- `getAvailableContracts()` возвращает вектор ПО ЗНАЧЕНИЮ: не хранить `&offer` из range-for по временному объекту (висячий указатель) — держать локальную копию.
- Имена параметров, совпадающие с namespace (economy, contracts), + пропущенный using = «недопустимое использование идентификатора пространства имён».
- Старые тестовые файлы, ссылающиеся на удалённые API, дают «переопределение/не является членом» — проверять актуальность файла при странных ошибках.
- doctest SUBCASE перезапускает тело теста на каждый сабкейс.

## 8. Структура файлов (актуальная)
```
Core/
├── CMakeLists.txt
├── data/ (goods.json, cities.json)
├── third_party/ (doctest/, nlohmann/)
├── include/kotr/{core,data,economy,contracts,finance,market,drivers,vehicles,leaderboard,licenses}/
├── src/{core,data,economy,contracts,finance,market,drivers,vehicles,leaderboard,licenses}/
└── tests/ (test_main, test_economy, test_data_loader, test_contract, test_market,
            test_drivers, test_driver_agents, test_vehicles, test_leaderboard).cpp
```