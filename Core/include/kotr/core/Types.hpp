#pragma once
#include <cstdint>
#include <string>

namespace kotr::core {

// === Единицы измерения (по DMS) ===

// Деньги: целые рубли, int64
using Money = int64_t;

// Время: игровые минуты, int64
using GameMinutes = int64_t;

// Вероятность: 0.0 – 1.0
using Probability = double;

// Репутация: -100 … +100
using Reputation = int32_t;

// === Entity ID ===

using EntityId = uint64_t;
constexpr EntityId INVALID_ENTITY_ID = 0;

// === Строковые ID (по DMS: snake_case, ASCII, префиксы) ===

using CityId         = std::string;  // "city_yuzhny"
using GoodId         = std::string;  // "good_food"
using DriverId       = std::string;  // "drv_..."
using VehicleModelId = std::string;  // "veh_model_..."
using VehicleInstId  = std::string;  // "veh_inst_..."
using FactionId      = std::string;  // "faction_kaimany"
using EventId        = std::string;  // "evt_bandit_ambush"
using ContractId     = std::string;  // "ctr_..."
using RouteId        = std::string;  // "route_m13"

} // namespace kotr::core