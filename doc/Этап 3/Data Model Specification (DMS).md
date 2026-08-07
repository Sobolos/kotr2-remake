# Data Model Specification (DMS)

**Проект:** Дальнобойщики 2 Remake  
**Версия:** 0.1  
**Статус:** Утверждён

## 1. Принципы
- **Static Content:** Справочники (города, товары, модели). Не меняются в рантайме.
- **Runtime State:** Текущее состояние (экономика, водители, контракты). Сохраняется.
- **Derived Data:** Расчётные данные (маршруты, риск). Могут пересчитываться.
- **History/Logs:** Журналы (финансы, события). Ограниченно сохраняются.

## 2. Соглашения
- **ID:** snake_case, ASCII, стабильные. Префиксы: `city_`, `good_`, `drv_`, `veh_model_`, `faction_`, `evt_`.
- **Локализация:** Ключи вида `loc.<домен>.<id>.<поле>`.
- **Теги:** `Домен.Категория.Значение` (например, `Cargo.Coal`, `Driver.Hidden.Thief`).
- **Единицы:** Длина (м), Масса (кг), Деньги (целые рубли, int64), Время (игровые минуты), Вероятность (0.0–1.0), Репутация (-100…+100).

## 3. Основные перечисления
- **EconomyMode:** Living, Classic
- **CitySpecialization:** Logistics, Mining, Forest, Oil, Agriculture, Metal, Port, Mountain, Transit
- **GoodCategory:** Raw, Industrial, Consumer, Fuel, Parts, Special, Illegal
- **ContractStatus:** Generated, Available, Accepted, Loading, InTransit, Arrived, Completed, Failed, Cancelled, Stolen, Betrayed
- **VehicleClass:** Light, Medium, Heavy, Special
- **DriverEmploymentStatus:** Unemployed, PlayerHired, Competitor, Neutral, Arrested, Incapacitated
- **FactionType:** Bandit, Police, Business, Civilian, Neutral
- **RiskEventCategory:** Bandit, Police, Weather, Breakdown, Driver, Traffic, World, Story

## 4. Ключевые сущности (выборочно)

### CityData / CityState
- Статика: специализация, производимые/потребляемые товары, базовая опасность, услуги.
- Динамика: текущие цены, запасы, доступность топлива/запчастей, контроль фракций.

### GoodData
- base_price_per_ton, cargo_value_per_ton, volatility, storage_class, danger_modifier, legal, perishable.

### ContractOffer / ContractState
- Offer: origin, destination, good, mass, payout, deadline, risk_tags, source.
- State: status, assigned_vehicle, assigned_driver, cargo_condition, progress.

### VehicleModelData / VehicleInstance
- Model: class, max_cargo_capacity_kg, fuel_consumption, reliability, towing_capacity.
- Instance: owner, condition, mileage, fuel, wear_parts, current_trailer, current_driver.
- *Примечание:* В долю рынка считается только `max_cargo_capacity_kg` тягача. Прицепы не учитываются.

### DriverPersonaData / DriverState
- Persona: personality_tags, hidden_tags, base_skills, license_requirements, salary_expectation.
- State: employment_status, employer_ref, loyalty, fatigue, morale, revealed_hidden_tags.

### RiskEventTemplate / Instance
- Template: category, severity, trigger_conditions, choices, outcomes, cooldown.
- Instance: template_ref, target, location, state, selected_choice.


### ThreatDirectorData / ThreatDirectorState
- active_threat_meter, recent_events, cooldowns, escalation_level, current_tension, last_major_event_time

### RiskPocket
- pocket_id, edge_ref, position, pocket_type, risk_tags, active_conditions, cooldown

### RepairPointData
- repair_id, edge_ref, position, repair_type, speed_limit, lane_closure, detour_ref, active_schedule, danger_modifier, police_modifier, bandit_modifier

### PoliceHeatState
- heat_value, wanted_level, recent_violations, last_fine_time, last_bribe_time, helicopter_unlocked

### BanditFeudState
- faction_ref, feud_level, last_conflict_time, debts, protection_contracts

### ServiceOrder
- order_id, provider_faction, service_type, target_ref, cost, success_chance, state, scheduled_time, outcome_ref

- DriverPersonaData: home_city, license_requirements, market_weight, vehicle_preference.
- DriverState: market_state, employer_ref, out_of_action_until, payment_policy_ref.
- EmploymentContract: license_required, vehicle_transfer, compensation_rules.
- LaborMarketState: active_visible_slots, rotation_queue, parked/independent lists.
- NamedCompetitorBusiness: money, business_level, hired_driver_refs, fleet_capacity.
- MassAICompetitor: capacity_kg, delivery_score, respawn_time.
- DeliveryLeaderboard: period, category, entries, winner, license_awarded.
- LicenseState: license_id, owner_ref, category, source_ranking.

### FactionData
faction_id, name_key, type, behavior_tags, aggression, greed, corruption, service_catalog_ref, preferred_payment, visual_tags, leaders, home_zones

### FactionState
faction_ref, global_power, territory_control, stance_player, active_wars, cooldowns

### ZoneControl
zone_ref, faction_ref, control_level, last_change_time

### FactionService
service_id, faction_ref, name_key, cost_base, required_reputation, risk_level, effect_tags, target_types, cooldown

### ServiceOrder
order_id, provider_faction, service_type, target_ref, cost, success_chance, state, scheduled_time, outcome_ref

### FactionWarState
war_id, participants, stage, tension, zones, start_time, duration, resolution

### DriverCommunityReputation
value, employer_score, rival_score, road_brother_score, tags

### ProtectionContract
contract_id, faction_ref, scope, cost, duration, effects

### BribeState
post_ref, corruption, personal_rate, cooldown

### EventTemplateData
event_id, name_key, category, type, priority, trigger_conditions, context_tags, choices, outcomes, cooldown, one_shot, weight, narrative_flags_set, narrative_flags_required

### EventInstance
instance_id, template_ref, timestamp, context_ref, state, selected_choice, outcomes_applied, narrative_flags_changed

### PersonalStoryData
personal_story_id, driver_ref, trigger_conditions, stages, choices, outcomes, narrative_flags

### StoryArcData
story_arc_id, name_key, stages, trigger_conditions, choices, outcomes, narrative_flags

### NarrativeState
story_arcs, personal_stories, narrative_flags, event_history, world_tension, faction_wars, player_legend

### EventCooldownState
template_ref, last_used_time, cooldown_override

### ContactMemory
contact_ref, trust_level, lies_detected, favors_owed, favors_given



## 5. Валидация
- Все ссылки валидны.
- Дорожный граф связен.
- Контракты имеют достижимые дедлайны.
- Грузоподъёмность >= массы груза.
- Скрытые теги водителей не видны в UI до раскрытия.