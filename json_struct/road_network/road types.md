### Класс A — Шестиполосное шоссе (Красный)
#### Базовые свойства
- class: A
- speed_limit_kmh: 110
- lanes_total: 6
- lanes_forward: 3
- lanes_backward: 3
- direction: bidirectional
- surface: asphalt
- has_intersections: false

#### Ограничения
- max_weight_t: 40
- max_axle_load_t: 10
- max_height_m: 5.0
- max_width_m: 3.0
- chokepoints: []

#### Риски
- base_risk: 0.05
- visibility: open
- traffic_density: 0.9
- curvature: 0.1
- patrol_frequency: 0.8
- risk_multipliers:
    weather_rain: 1.3
    weather_snow: 2.0
    weather_fog: 1.5
    night: 1.2
    faction_hostile: 1.5
    convoy_heavy: 1.1

#### Геометрия
- slope_profile:
    - {at: 0.0, grade: 0.0}
    - {at: 1.0, grade: 0.0}
- banking_angle_max: 3
- escape_routes: []

#### Геймплей
- ownership: state
- toll: 0
- seasonal:
    winter: passable
    spring: passable
    summer: passable
    autumn: passable
- tags: [highway, major]

---

### Класс B — Четырёхполосное шоссе (Розовый)
#### Базовые свойства
- class: B
- speed_limit_kmh: 90
- lanes_total: 4
- lanes_forward: 2
- lanes_backward: 2
- direction: bidirectional
- surface: asphalt
- has_intersections: true

#### Ограничения
- max_weight_t: 40
- max_axle_load_t: 10
- max_height_m: 5.0
- max_width_m: 3.0
- chokepoints: []

#### Риски
- base_risk: 0.1
- visibility: open
- traffic_density: 0.7
- curvature: 0.25
- patrol_frequency: 0.6
- risk_multipliers:
    weather_rain: 1.4
    weather_snow: 2.2
    weather_fog: 1.6
    night: 1.3
    faction_hostile: 1.7
    convoy_heavy: 1.15

#### Геометрия
- slope_profile:
    - {at: 0.0, grade: 0.0}
    - {at: 1.0, grade: 0.0}
- banking_angle_max: 4
- escape_routes: []

#### Геймплей
- ownership: state
- toll: 0
- seasonal:
    winter: passable
    spring: passable
    summer: passable
    autumn: passable
- tags: [highway, regional]

---

### Класс C — Широкое двуполосное шоссе (Бирюзовый)
#### Базовые свойства
- class: C
- speed_limit_kmh: 90
- lanes_total: 2
- lanes_forward: 1
- lanes_backward: 1
- direction: bidirectional
- surface: asphalt
- has_intersections: false

#### Ограничения
- max_weight_t: 30
- max_axle_load_t: 9
- max_height_m: 4.5
- max_width_m: 2.8
- chokepoints: []

#### Риски
- base_risk: 0.2
- visibility: open
- traffic_density: 0.4
- curvature: 0.4
- patrol_frequency: 0.4
- risk_multipliers:
    weather_rain: 1.5
    weather_snow: 2.5
    weather_fog: 1.8
    night: 1.4
    faction_hostile: 2.0
    convoy_heavy: 1.2

#### Геометрия
- slope_profile:
    - {at: 0.0, grade: 0.0}
    - {at: 0.5, grade: 0.03}
    - {at: 1.0, grade: -0.02}
- banking_angle_max: 5
- escape_routes: []

#### Геймплей
- ownership: state
- toll: 0
- seasonal:
    winter: passable
    spring: passable
    summer: passable
    autumn: passable
- tags: [regional, scenic]

---

### Класс D — Узкое двуполосное шоссе (Фиолетовый)
#### Базовые свойства
- class: D
- speed_limit_kmh: 70
- lanes_total: 2
- lanes_forward: 1
- lanes_backward: 1
- direction: bidirectional
- surface: asphalt
- has_intersections: true

#### Ограничения
- max_weight_t: 20
- max_axle_load_t: 7
- max_height_m: 4.0
- max_width_m: 2.5
- chokepoints: []

#### Риски
- base_risk: 0.35
- visibility: forest
- traffic_density: 0.25
- curvature: 0.5
- patrol_frequency: 0.3
- risk_multipliers:
    weather_rain: 1.6
    weather_snow: 2.8
    weather_fog: 2.0
    night: 1.6
    faction_hostile: 2.2
    convoy_heavy: 1.3

#### Геометрия
- slope_profile:
    - {at: 0.0, grade: 0.0}
    - {at: 0.3, grade: 0.06}
    - {at: 0.7, grade: -0.04}
    - {at: 1.0, grade: 0.0}
- banking_angle_max: 6
- escape_routes: []

#### Геймплей
- ownership: private
- toll: 0
- seasonal:
    winter: difficult
    spring: muddy
    summer: passable
    autumn: muddy
- tags: [country, narrow]

---

### Класс E — Грунтовая двуполосная узкая (Белый)
#### Базовые свойства
- class: E
- speed_limit_kmh: 50
- lanes_total: 2
- lanes_forward: 1
- lanes_backward: 1
- direction: bidirectional
- surface: gravel
- has_intersections: true

#### Ограничения
- max_weight_t: 15
- max_axle_load_t: 6
- max_height_m: 3.5
- max_width_m: 2.2
- chokepoints: []

#### Риски
- base_risk: 0.5
- visibility: forest
- traffic_density: 0.1
- curvature: 0.6
- patrol_frequency: 0.1
- risk_multipliers:
    weather_rain: 2.0
    weather_snow: 3.5
    weather_fog: 2.5
    night: 2.0
    faction_hostile: 2.8
    convoy_heavy: 1.5

#### Геометрия
- slope_profile:
    - {at: 0.0, grade: 0.0}
    - {at: 0.4, grade: 0.08}
    - {at: 0.8, grade: -0.06}
    - {at: 1.0, grade: 0.0}
- banking_angle_max: 4
- escape_routes: []

#### Геймплей
- ownership: neutral
- toll: 0
- seasonal:
    winter: difficult
    spring: muddy
    summer: passable
    autumn: muddy
- tags: [dirt, rural, remote]

---

### Класс S — Скрытая дорога (Жёлтый)
#### Базовые свойства
- class: S
- speed_limit_kmh: 40
- lanes_total: 1
- lanes_forward: 1
- lanes_backward: 0
- direction: oneway_forward
- surface: dirt
- has_intersections: false

#### Ограничения
- max_weight_t: 10
- max_axle_load_t: 5
- max_height_m: 3.0
- max_width_m: 2.0
- chokepoints: []

#### Риски
- base_risk: 0.7
- visibility: forest
- traffic_density: 0.02
- curvature: 0.8
- patrol_frequency: 0.0
- risk_multipliers:
    weather_rain: 2.5
    weather_snow: 4.0
    weather_fog: 3.0
    night: 2.5
    faction_hostile: 3.0
    convoy_heavy: 2.0

#### Геометрия
- slope_profile:
    - {at: 0.0, grade: 0.0}
    - {at: 0.5, grade: 0.1}
    - {at: 1.0, grade: 0.0}
- banking_angle_max: 3
- escape_routes: []

#### Геймплей
- ownership: unknown
- toll: 0
- seasonal:
    winter: muddy
    spring: muddy
    summer: difficult
    autumn: difficult
- tags: [secret, hidden, dangerous]