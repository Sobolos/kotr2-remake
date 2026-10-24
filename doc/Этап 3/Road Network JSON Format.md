# Road Network JSON Format (приложение к Road Network & World Design)

**Проект:** Дальнобойщики 2 Remake
**Версия:** 0.1
**Статус:** Черновик
**Назначение:** Формализация структуры файлов `json_struct/road_network/` для сборки графа дорог, расчёта маршрутов и рисков, простановки POI и интеграции с Unreal Engine.

---

## 1. Принципы описания

1. **Данные отдельно от кода.** Все параметры дорог — в JSON (TAD: data-driven).
2. **Справочники выносятся в отдельные файлы.** Типы дорог (`road_types.json`) — статический справочник; конкретные участки (`network.json`) ссылаются на них по ID. Изменение характеристик класса дороги (лимит скорости, базовая опасность) не требует правки графа.
3. **Один источник истины для геометрии.** Core-граф оперирует узлами и рёбрами с длиной; визуальная кривая (сплайн/дорожка Map) живёт в UE и связывается с ребром по `edge_id`.
4. **Позиции объектов на дороге задаются параметрически** — `(edge_ref, position_m)` либо нормированной координатой `t ∈ [0..1]`, а НЕ абсолютными XY. Это позволяет двигать сплайны в UE без рассинхрона точек_interest/рисков.
5. **Единицы (по DMS §2):** длина/позиции — метры (м), массы — кг, скорость — км/ч, вероятность/опасность — 0.0–1.0, время — игровые минуты.
6. **ID:** snake_case, ASCII, стабильные. Префиксы: `rnode_`, `redge_`, `rtype_`, `poi_`, `rpnt_` (ремонтная точка), `pocket_` (риск-карман), `zone_`, `detour_`.

---

## 2. Каталог файлов

```
json_struct/road_network/
├── road_types.json      # Справочник классов дорог A–F (статика)
├── network.json         # Граф: узлы + рёбра (статика контента)
├── poi.json             # Точки интереса (базы, заправки, СТО, посты)
├── risk_points.json     # RiskPocket: потенциальные места засад/перекрытий
├── repair_slots.json    # Потенциальные ремонтные слоты на рёбрах
└── zones.json           # Зоны влияния фракций (связь с FactionSystem)
```

Все файлы содержат `"version": "0.1"` и `"schema": "<имя схемы>"` для будущей миграции сохранений/данных.

---

## 3. road_types.json — справочник классов дорог

Соответствует §3 Road Network Design.

```json
{
  "version": "0.1",
  "schema": "kotr.road_types.v1",
  "types": [
    { "id": "rtype_a", "name_key": "loc.road.type.a", "class": "A",
      "width_m": 16, "speed_limit_kmh": 90, "danger_base": 0.20,
      "surface_default": "asphalt", "traffic_density": 0.7,
      "repair_slot_range": [2, 4], "tags": ["Route.Major"] },
    { "id": "rtype_b", "class": "B", "width_m": 8, "speed_limit_kmh": 75,
      "danger_base": 0.30, "surface_default": "asphalt", "traffic_density": 0.5,
      "repair_slot_range": [2, 3] },
    { "id": "rtype_c", "class": "C", "width_m": 6, "speed_limit_kmh": 60,
      "danger_base": 0.35, "surface_default": "asphalt_bad", "traffic_density": 0.3,
      "repair_slot_range": [1, 2] },
    { "id": "rtype_d", "class": "D", "width_m": 5, "speed_limit_kmh": 45,
      "danger_base": 0.45, "surface_default": "dirt", "traffic_density": 0.15,
      "repair_slot_range": [1, 2] },
    { "id": "rtype_e", "class": "E", "width_m": 4, "speed_limit_kmh": 45,
      "danger_base": 0.55, "surface_default": "mountain", "traffic_density": 0.1,
      "repair_slot_range": [2, 2] },
    { "id": "rtype_f", "class": "F", "width_m": 4, "speed_limit_kmh": 40,
      "danger_base": 0.60, "surface_default": "unpaved", "traffic_density": 0.05,
      "repair_slot_range": [0, 1], "tags": ["Route.Secret"] }
  ]
}
```

Поля:
- `class` — A/B/C/D/E/F (Road Network Design §3).
- `speed_limit_kmh` — верхняя граница диапазона класса; конкретное ребро может сузить (`speed_limit_override`).
- `danger_base` — базовая опасность класса (используется в RouteRisk, Risk System §6).
- `repair_slot_range` — число потенциальных ремонтных слотов на ребре (Risk System §20.1).
- `traffic_density` — базовый трафик для EnvironmentRisk/TrafficMod.

---

## 4. network.json — граф

### 4.1 Узлы (RoadNode)

Три типа узлов: `city` (привязан к городу из cities.json), `junction` (перекрёсток/развилка), `waypoint` (промежуточная точка перегиба геометрии — нужен только для точности сплайнов, в логике не участвует).

```json
{
  "nodes": [
    { "id": "rnode_yuzhny",      "type": "city",     "city_ref": "city_yuzhny",
      "x_m": 4500, "y_m": 3000, "tile": "x04_y03" },
    { "id": "rnode_elnino",      "type": "city",     "city_ref": "city_elnino",
      "x_m": 9500, "y_m": 3500, "tile": "x09_y03" },
    { "id": "rnode_j_m13_07",    "type": "junction",
      "x_m": 12000, "y_m": 6500, "tile": "x12_y06" },
    { "id": "rnode_wp_m13_12",   "type": "waypoint", "x_m": 13500, "y_m": 7000 }
  ]
}
```

- `x_m, y_m` — мировые координаты в метрах (мир 20×20 км, X запад→восток, Y юг→север).
- `tile` — тайл World Partition 1×1 км (для стриминга и отладки).
- Для `city`-узлов `city_ref` обязан существовать в `cities.json`; координаты сверяются с таблицей §2 Road Network Design.

### 4.2 Рёбра (RoadEdge)

```json
{
  "edges": [
    {
      "id": "redge_m13_yuzhny_elnino",
      "from": "rnode_yuzhny",
      "to": "rnode_elnino",
      "road_type_ref": "rtype_a",
      "route_refs": ["route_m13"],
      "length_m": 5100,
      "bidirectional": true,
      "one_way": false,
      "surface": "asphalt",
      "condition": 0.8,
      "speed_limit_override_kmh": null,
      "danger_override": null,
      "zone_refs": ["zone_south_police"],
      "geometry_ref": "spline_road_m13_01",
      "waypoints_between": ["rnode_wp_m13_02", "rnode_wp_m13_03"]
    }
  ]
}
```

Поля:
- `from`/`to` — ID узлов. Ребро — прямое; двунаправленность флагом `bidirectional` (для A/B/M-маршрутов true; для опасных горных/секретных можно false, создавая односторонние «мышиные» тропы).
- `length_m` — длина вдоль дороги (не евклидова!). Валидируется: `|length_m − euclid(from,to)| ≤ +15%` для прямых; для серпантинов допускается большее, но проверяется ≥ евклидовой.
- `surface` — при необходимости переопределяет `surface_default` из справочника (например, разбитый асфальт на участке A-класса).
- `condition` 0..1 — стартовое состояние покрытия (множитель surface_mod в RouteRisk).
- `danger_override` — если участок аномально опасен/безопасен относительно класса.
- `zone_refs` — ссылки на зоны влияния из `zones.json` (§7).
- `geometry_ref` — имя сплайна/дорожки в UE (Level Instance / Map). Core не использует, служит мостом для интеграции (§8).
- `route_refs` — принадлежность ключевым маршрутам (§4 Road Network Design): `route_m13`, `route_profit_corridor`, секреты: `route_old_mine`, `route_forest_logging`, `route_smuggler_trail`, `route_dead_pass`.

---

## 5. poi.json — точки интереса

Типы POI (по документации): `commodity_base` (товарная база), `fuel_depot` (нефтебаза/заправка), `service_station` (СТО), `police_post` (пост милиции), `bandit_camp`, `market`, `port`, `race_ring` (Кольцо), `custom_poi`.

```json
{
  "pois": [
    { "id": "poi_yuzhny_base_food", "type": "commodity_base",
      "anchor": { "edge_ref": "redge_m13_yuzhny_elnino", "position_m": 300 },
      "city_ref": "city_yuzhny",
      "goods": ["good_food", "good_timber"],
      "contract_source": true,
      "services": ["load", "unload", "contract_board"],
      "name_key": "loc.poi.yuzhny_base_food.name" },
    { "id": "poi_yuzhny_fuel_depot", "type": "fuel_depot",
      "anchor": { "node_ref": "rnode_yuzhny", "offset_m": [120, -80] },
      "city_ref": "city_yuzhny",
      "contract_source": true,
      "fuel_capacity_l": 20000,
      "services": ["refuel"] },
    { "id": "poi_klyuchi_sto", "type": "service_station",
      "anchor": { "edge_ref": "redge_m13_elnino_klyuchi", "t": 0.55 },
      "city_ref": "city_klyuchi",
      "services": ["repair", "parts"] }
  ]
}
```

Правила:
- **Якорение только через граф** (`edge_ref + position_m` или `edge_ref + t`, либо `node_ref + offset_m`). Абсолютные координаты в POI запрещены — их вычисляет loader.
- `contract_source: true` — POI может выдавать контракты. Источников контрактов ровно два вида: **нефтебаза** (`fuel_depot`) и **товарная база в городе** (`commodity_base`). Ничего другого контракты не даёт (согласование пользователя 25.10.2026).
- Товарные базы привязаны к городам (`city_ref`) и к товарам из `goods.json`; привоз/вывоз резонирует с `EconomySystem.deliverGood`.
- Полицейские посты влияют на `PoliceHeat` (радиус действия — отдельный параметр `radius_m`), бандитские лагеря — на BanditPressure и являются якорями засад.

---

## 6. risk_points.json — RiskPocket

Соответствует DMS §RiskPocket и Risk System Design (засады, перекрытия).

```json
{
  "pockets": [
    { "id": "pocket_zalessk_forest_01",
      "edge_ref": "redge_c_zalessk_klyuchi",
      "position_m": 2400,
      "pocket_type": "ambush",
      "risk_tags": ["Bandit.Ambush", "Route.Forest"],
      "base_chance": 0.35,
      "active_conditions": { "night_only": true, "min_bandit_pressure": 0.35 },
      "cooldown_min_game_minutes": 1440,
      "linked_event_template": "evt_bandit_radio_warning" },
    { "id": "pocket_shakhty_roadblock_01",
      "edge_ref": "redge_m13_klyuchi_shakhty",
      "t": 0.7,
      "pocket_type": "roadblock",
      "risk_tags": ["Police.Checkpoint"],
      "base_chance": 0.9,
      "active_conditions": { "always": true },
      "cooldown_min_game_minutes": 0,
      "linked_event_template": "evt_police_checkpoint" }
  ]
}
```

- `pocket_type`: `ambush`, `roadblock`, `checkpoint`, `improvised_trap`, `breakdown_hotspot`.
- `linked_event_template` — ссылка на шаблон события Risk System (§32 примеры событий); событие генерирует ThreatDirector при проезде игроком позиции pocket.
- Поломки (Breakdown) к pocket НЕ привязываются — они считаются от состояния машины и условий ребра (решение пользователя); для горных зон задаётся тег `Route.Mountain`, который входит в формулу VehicleRisk.

---

## 7. zones.json — зоны влияния фракций

Мост к FactionSystem (ZoneControl из DMS). Зона описывается списком рёбер и базовым контролем.

```json
{
  "zones": [
    { "id": "zone_zalessk_kaimany",
      "faction_ref": "faction_kaimany",
      "edges": ["redge_c_zalessk_klyuchi", "redge_d_zalessk_borovoe_secret"],
      "control_level": 0.7,
      "danger_modifier": 0.15,
      "police_modifier": -0.2,
      "bandit_modifier": 0.3 },
    { "id": "zone_south_police",
      "faction_ref": "faction_police",
      "edges": ["redge_m13_yuzhny_elnino"],
      "control_level": 0.6,
      "police_modifier": 0.25 }
  ]
}
```

`FactionPressure = max(faction_aggression × zone_control × route_relevance)` (Risk System §10.1) считается по рёбрам маршрута через эти данные.

---

## 8. Интеграция с Unreal Engine

1. **Core остаётся источником логики**: маршруты (Dijkstra/A* по графу), риск, расстояния, ETA контрактов.
2. **Генератор UE-геометрии (Editor-скрипт/тулза на Python)** читает `network.json` и создаёт:
   - один Level Instance на тайл 1×1 км (World Partition, §6 Road Network Design);
   - Road Spline Component на каждый `redge_*` с именем из `geometry_ref`; waypoint-узлы графа становятся контрольными точками сплайна;
   - акторы POI/pocket/repair на сплайне по `position_m`/`t` (Spline Mesh / Child Actor Component).
3. **Обратный пилотаж**: если дизайнер двигал сплайн в редакторе, тулза пересчитывает `length_m` рёбер и пишет патч в `network.json` (или отдельный `network_geometry_patch.json`), после чего запускается валидатор графа (§9).
4. **Сопоставление runtime-позиций**: у машины в UE есть `edge_id + distance_along_edge` (ближайшая точка сплайна). Этот же `(edge_ref, position_m)` понимает Core — так синхронизируются риски, ремонты и POI без пересчёта координат.
5. Тайл-ключ (`tile`) в узлах/рёбрах позволяет фильтровать контент при загрузке cell'а World Partition.

---

## 9. Валидатор графа (обязательный шаг после каждой правки)

Тест `KotRTestRoadNetwork` (Core/tests) проверяет:
1. Все ссылки существуют: `from/to`, `road_type_ref`, `city_ref`, `edge_ref`, `zone_refs`, `faction_ref`, `linked_event_template`.
2. Граф **связен** (DMS §5 «Дорожный граф связен») — неориентированная связность всех city-узлов; проверка сильной связности для односторонних рёбер опциональна (предупреждение).
3. `length_m ≥ euclid(from,to)` и отсутствие self-loop/дублей рёбер.
4. Каждое ключевое место доступно: все 12 городов §2 имеют хотя бы одно incident-ребро; М-13 проходит цепочкой Южный→Ельнино→Ключи→Шахты→Приозёрск→Нижнегорск.
5. Каждый город имеет минимум один `commodity_base` и один `fuel_depot` (источники контрактов и топлива).
6. У каждого ребра есть покрытие классом A/B (нет изолированных грунтовок без связи) — предупреждение.
7. Позиции pocket/repair/POI лежат внутри `[0, length_m]` своего ребра.
8. Слоты ремонтов на ребре соответствуют `repair_slot_range` класса.

---

## 10. Порядок дальнейшей работы (RoadNetwork)

1. Заполнить `network.json` по PNG-карте: нанести все junction/waypoint, собрать рёбра, проверить длины (см. §11 методика обмера).
2. Прогнать валидатор (шаги 1–3), затем расширенный (4–8) после появления poi/zones.
3. `poi.json`: расставить товарные базы, нефтебазы, СТО, заправки, посты по городам и дорогам.
4. `zones.json` + `risk_points.json`: разметить зоны фракций и потенциальные точки засад/перекрытий (активируются ThreatDirector'ом в рантайме).
5. В Core: модуль `kotr::roads` — структуры `RoadNode/RoadEdge/RoadType/PoiRef/RiskPocket/RepairSlot`, `RoadGraphLoader` (JSON → память), `RouteService` (Dijkstra: длина, время в пути, профиль риска) и перевод `ContractSystem`/`DriverAgentSystem` с евклидовых расстояний на граф (PROJECT_STATUS §4.C, Фаза 9).
6. Тулза экспорта в UE (§8.2) — когда граф заморожен.

---

## 11. Методика переноса с топографической PNG-карты

1. **Калибровка:** мир 20×20 км. Найти на карте 2 дальних опорных объекта (города с известными координатами из §2 Road Network Design) и измерить расстояние между ними в пикселях → `m_per_px`. Проверить третью точку (невязка ≤ 2%).
2. **Векторизация:** в любом редакторе (Inkscape/QGIS/Junction-плагин) обвести дороги полилиниями; перекрёстки — узлы. Не повторять каждую извилину: waypoint ставится там, где кривизна меняет длину участка >2%.
3. **Экспорт:** координаты пиксели → метры через калибровку; сохранить промежуточный CSV/GeoJSON; скриптом (Python) сгенерировать `network.json` (id по схеме `redge_<класс>_<from>_<to>`).
4. **Классификация участков:** A/B/C/D/E/F по внешнему виду карты (толщина/тип штриха), секретные F — вручную по лору (4 срезки §4).
5. **Обмер длины:** `length_m` = длина полилинии × m_per_px (не расстояние между центрами!).
6. **Расстановка POI/risks:** поверх готового графа, параметрически (§5, §6) — сначала обязательные (базы/заправки в каждом городе), потом факультативные (засады).
7. После каждого изменения — валидатор §9.
