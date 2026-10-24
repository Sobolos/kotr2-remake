## Рёбра
### М1
- id: edge_a_m1_01
  - from: north-west_map_border
  - to: turbo_m1_e1_m1
  - class: A
  - length_km: 3
  - note: Основная трасса М1. Участок от границы карты до развязки с Е1
- id: edge_a_m1_02
  - from: turbo_m1_e1_m1
  - to: turbo_m1_m2_m1
  - class: A
  - length_km: 3.2
  - note: Основная трасса М1. Участок между развязками turbo_m1_e1_m1 и turbo_m1_m2_m1
  - poi:
    - gas_station_m1_01
      - name: gas_station_m1_01
      - type: gas_station
      - location: 0.45
      - side: right
    - road_blocked_m1_01
      - name: line_blocked_m1
      - type: road_repairing
      - location: 0.35
      - side: left
    - police
      - id:police_01_edge_a_m1_02
      - position: 0.48
      - side: right
      - type: police_car
  - obstacles:
    - roadwork
      - id: roadwork_01_edge_a_m1_02
      - position: 0.3
      - side: left
    - roadwork
      - id: roadwork_02_edge_a_m1_02
      - position: 0.35
      - side: right
- id: edge_a_m1_03
  - from: turbo_m1_m2_m1
  - to: bridge_a_m1_1_1
  - class: A
  - length_km: 3
  - note: Основная трасса М1. Участок между развязками turbo_m1_e1_m1 и мостом bridgeM1_1
  - poi:
      - police_m1_03
        - type: police_car
        - location: 0.40
        - side: left
  - obstacles:
      - roadwork_m1_03
        - type: roadwork
        - location: 0.70
        - side: right
        - severity: medium
- id: bridge_a_m1_1
  - from: bridge_a_m1_1_1
  - to: bridge_a_m1_1_2
  - class: A
  - length_km: 0.5
  - note: Мост на трассе М1
- id: edge_a_m1_04
  - from: bridge_a_m1_1_2
  - to: bridge_a_m1_2_1
  - class: A
  - length_km: 7
  - note: Участок между мостами bridgeM1_1 и bridgeМ1_2 на трассе М1
  - poi:
    - gas_station_m1_02
      - name: gas_station_m1_02
      - type: gas_station
      - location: 0.15
      - side: right
    - gas_station_m1_03
      - name: gas_station_m1_03
      - type: gas_station
      - location: 0.25
      - side: left
    - police_01_edge_a_m1_04
      - position: 0.5
      - side: left
      - type: post
    - police_01_edge_a_m1_04
      - position: 0.5
      - side: left
      - type: post
  - obstacles:
      - roadwork_m1_04_a
        - type: roadwork
        - location: 0.65
        - side: right
        - severity: low
- id: bridge_a_m1_2
  - from: bridge_a_m1_2_1
  - to: bridge_a_m1_2_2
  - class: A
  - length_km: 0.7
  - note: Мост на трассе М1
  - obstacles:
    - roadwork
      - id: roadwork_01_bridge_a_m1_2
      - position: 0.45
      - side: left
    - roadwork
      - id: roadwork_02_bridge_a_m1_2
      - position: 0.65
      - side: right
- id: edge_a_m1_05
  - from: bridge_a_m1_2_2
  - to: turbo_m1_m1_m1
  - class: A
  - length_km: 4.5
  - note: Развязка на Юго-восточный выезд из региона
  - poi:
    - service_station_01
      - name: service_station_m1_01
      - type: service_station
      - location: 0.1
      - side: right
    - gas_station_04
      - name: gas_station_m1_04
      - type: gas_station
      - location: 0.9
      - side: right
    - gas_station_05
      - name: gas_station_m1_05
      - type: gas_station
      - location: 0.8
      - side: left
- id: edge_a_m1_07
  - from: turbo_m1_m1_m1
  - to: north-east_map_border
  - class: A
  - length_km: 1.6
  - note: Развязка на Юго-восточный выезд из региона
- id: edge_a_m1_06
  - from: turbo_m1_m1_m1
  - to: turbo_m1_m2_m3
  - class: A
  - length_km: 4.2
  - note: Развязка на Нижнегорск
  - obstacles:
    - crash
      - crash_01_edge_a_m1_06
        - position: 0.05
        - side: left
    - crash
      - crash_02_edge_a_m1_06
        - position: 0.05
        - side: right
  - poi:
      - police_m1_06
        - type: police_car
        - location: 0.75
        - side: right
### М3
- id: edge_b_m3_01
  - from: turbo_m1_m2_m3
  - to: bridge_b_m3_1_1
  - class: B
  - length_km: 0.5
  - note: Восточный въезд на мост на трассе М2

- id: bridge_b_m3_1
  - from: bridge_b_m3_1_1
  - to: bridge_b_m3_1_2
  - class: B
  - length_km: 0.3
  - note: мост на трассе М2

- id: edge_b_m3_02
  - from: bridge_b_m3_1_2
  - to: t-cross_m3_e4
  - class: B
  - length_km: 2.3
  - note: Нерегулируемый поворот на Подгорск
  - poi: 
    - police_01_edge_b_m3_02
      - position: 0.9
      - side: right
      - type: post

- id: edge_b_m3_03
  - from: t-cross_m3_e4
  - to: t-cross_m3_s1
  - class: B
  - length_km: 4.5
  - note: Секретная дорога на горный
  - obstacles:
      - road_blocked_m3_03
        - type: road_blocked
        - location: 0.35
        - side: left
        - note: временное перекрытие (ремонт)
        - duration_hours: 4-12

- id: edge_b_m3_04
  - from: t-cross_m3_s1
  - to: borovoe_gate_1_east
  - class: B
  - length_km: 1.5
  - note: Восточный въезд в Боровое

- id: edge_c_m3_05
  - from: borovoe_gate_1_west
  - to: t-cross_m3_s2
  - class: C
  - length_km: 5
  - note: Западный въезд в Боровое - Секретная дорога в горы

- id: edge_c_m3_06
  - from: t-cross_m3_s2
  - to: shahty_east_gate_1
  - class: C
  - length_km: 11
  - note: Восточный въезд в Шахты
  - poi:
    - gas_station_b_01
      - name: gas_station_b_01
      - type: gas_station
      - location: 0.15
      - side: left
    - service_station_b_01
      - name: service_station_b_01
      - type: service_station
      - location: 0.45
      - side: left
  - poi:
      - police_m3_06
        - type: police_car
        - location: 0.30
        - side: right
  - obstacles:
      - roadwork_m3_06_a
        - type: roadwork
        - location: 0.60
        - side: left
        - severity: medium
      - roadwork_m3_06_b
        - type: roadwork
        - location: 0.85
        - side: right
        - severity: low

- id: edge_b_m3_07
  - from: shahty_west_gate_1
  - to: bridge_b_m3_2_1
  - class: B
  - length_km: 8
  - note: Западный въезд в Шахты
  - poi:
      - police_m3_07
        - type: police_post
        - location: 0.45
        - side: left
  - obstacles:
      - road_blocked_m3_07
        - type: road_blocked
        - location: 0.20
        - side: right

- id: bridge_c_m3_2
  - from: bridge_b_m3_2_1
  - to: bridge_b_m3_2_2
  - class: B
  - length_km: 0.25
  - note: Мост на Шахты

- id: edge_b_m3_08
  - from: bridge_b_m3_2_2
  - to: x-cross-m3-m2-1
  - class: B
  - length_km: 5
  - note: перекресток Шахты-Южный-Приозерск-Залесск
  - obstacles:
      - roadwork_m3_08
        - type: roadwork
        - location: 0.55
        - side: left

- id: edge_b_m3_09
  - from: x-cross-m3-m2-1
  - to: t-cross-m3-m2-2
  - class: B
  - length_km: 0.5
  - note: перекресток Шахты-Южный-Приозерск-Залесск
  - poi:
    - gas_station_b_02
      - name: gas_station_b_02
      - type: gas_station
      - location: 0.2
      - side: right
    - service_station_b_03
      - name: service_station_b_03
      - type: service_station
      - location: 0.8
      - side: right
    - parking_01
      - name: parking_b_01
      - type: parking
      - location: 0.5
      - side: left

- id: edge_b_m3_10
  - from: t-cross-m3-m2-2
  - to: t-cross-m3-e1
  - class: B
  - length_km: 3
  - note: перекресток Южный-Залесск-шахты
  - poi:
      - police_m3_10
        - type: police_car
        - location: 0.40
        - side: right

### М2

- id: edge_b_m2_01
  - from: west_north_end_map
  - to: t_cross_m2_e1
  - class: B
  - length_km: 0.625
  - note: Граница региона на Юго-Западе

- id: edge_b_m2_02
  - from: t_cross_m2_e1
  - to: west_yuzhniy_gate_01
  - class: B
  - length_km: 0.355
  - note: Западный въезд в южный

- id: edge_b_m2_03
  - from: east_yuzhniy_gate_01
  - to: south_dam_01_01
  - class: B
  - length_km: 3.5
  - note: Западный въезд на дамбу
  - obstacles:
      - roadwork_m2_03
        - type: roadwork
        - location: 0.50
        - side: left

- id: south_dam_01
  - from: south_dam_01_01
  - to: south_dam_01_02
  - class: B
  - length_km: 0.825
  - note: ГЭС к западу от Южного

- id: edge_b_m2_04
  - from: south_dam_01_02
  - to: x-cross-m3-m2-1
  - class: B
  - length_km: 1.25
  - note: перекресток Шахты-Южный-Приозерск-Залесск

- id: edge_b_m2_05
  - from: x-cross-m3-m2-1
  - to: t-cross-m2-m2-1
  - class: B
  - length_km: 0.5
  - note: перекресток Шахты-Южный-Приозерск-Залесск
  - poi:
    - service_station_m2_01
      - name: service_station_m2_01
      - type: service_station
      - location: 0.8
      - side: left
    - parking_m2_01
      - name: parking_m2_01
      - type: parking
      - location: 0.5
      - side: left

- id: edge_b_m2_06
  - from: t-cross-m2-m2-1
  - to: t-cross-m3-m2-2
  - class: B
  - length_km: 0.3
  - note: перекресток Шахты-Южный-Приозерск-Залесск

- id: edge_b_m2_07
  - from: t-cross-m2-m2-1
  - to: bridge_b_m2_01_01
  - class: B
  - length_km: 3
  - note: южный въезд мост на приозерск
  - poi:
    - parking_m2_02
        - name: parking_m2_02
        - type: parking
        - location: 0.075
        - side: right
    - police_m2_07
      - type: police_car
      - location: 0.60
      - side: right

- id: bridge_b_m2_01
  - from: bridge_b_m2_01_01
  - to: bridge_b_m2_01_02
  - class: B
  - length_km: 0.2
  - note: мост на приозерск

- id: edge_b_m2_08
  - from: bridge_b_m2_01_02
  - to: bridge_b_m2_02_01
  - class: B
  - length_km: 1.8
  - note: между мостами

- id: bridge_b_m2_02
  - from: bridge_b_m2_02_01
  - to: bridge_b_m2_02_02
  - class: B
  - length_km: 0.2
  - note: мост на приозерск

- id: edge_b_m2_09
  - from: bridge_b_m2_02_02
  - to: Priozersk_south_gate_01
  - class: B
  - length_km: 1.2
  - note: южный въезд в Приозерск

- id: edge_b_m2_10
  - from: Priozersk_north-east_gate_01
  - to: brige_b_m2_03_1
  - class: A
  - length_km: 8
  - note: Северо-восточный выезд из Приозерска
  - poi:
    - gas_station_m2_02
      - name: gas_station_m2_02
      - type: gas_station
      - location: 0.85
      - side: left
    - parking_m2_03
      - name: parking_m2_03
      - type: parking
      - location: 0.75
      - side: left
    - parking_m2_04
      - name: parking_m2_04
      - type: parking
      - location: 0.2
      - side: right
    - service_station_m2_02
      - name: service_station_m2_02
      - type: service_station
      - location: 0.4
      - side: right
    - parking_m2_05
      - name: parking_m2_05
      - type: parking
      - location: 0.45
      - side: right
    - parking_m2_06
      - name: parking_m2_06
      - type: parking
      - location: 0.45
      - side: left
    - police_m2_10_a
        - type: police_post
        - location: 0.25
        - side: left
    - police_m2_10_b
        - type: police_car
        - location: 0.70
        - side: right
  - obstacles:
    - roadwork_m2_10
      - type: roadwork
      - location: 0.50
      - side: left
      - severity: medium

- id: brige_b_m2_03
  - from: brige_b_m2_03_1
  - to: brige_b_m2_03_2
  - class: A
  - length_km: 0.5
  - note: Мост на трассе М2

- id: edge_b_m2_11
  - from: brige_b_m2_03_2
  - to: t_cross_m2_s4
  - class: A
  - length_km: 0.2
  - note: Секретная дорога на е3

- id: edge_b_m2_12
  - from: t_cross_m2_s4
  - to: intersection_m2_e4_01
  - class: A
  - length_km: 1
  - note: Развязка на подгорск
  - poi: 
   - parking_07
      - name: parking_07
      - type: parking
      - location: 0.95
      - side: left
    - gas_station_10
      - name: gas_station_10
      - type: gas_station
      - location: 0.85
      - side: right
  - obstacles:
      - road_blocked_m2_12
        - type: road_blocked
        - location: 0.40
        - side: right

- id: edge_b_m2_13
  - from: intersection_m2_e4_01
  - to: brige_b_m2_04_1
  - class: A
  - length_km: 2.5
  - note: мост на М2 к югу от Бухты
  - poi: 
   - parking_808
      - name: parking_808
      - type: parking
      - location: 0.95
      - side: right
    - police_m2_13
      - type: police_car
      - location: 0.55
      - side: left
- id: brige_b_m2_04
  - from: brige_b_m2_04_1
  - to: brige_b_m2_04_2
  - class: A
  - length_km: 0.7
  - note: мост на М2 к югу от Бухты

- id: edge_b_m2_14
  - from: brige_b_m2_04_2
  - to: buhta_south_gate_01
  - class: A
  - length_km: 0.1
  - note: Южный въезд в Бухту

- id: edge_b_m2_15
  - from: buhta_north_gate_01
  - to: brige_b_m2_05_1
  - class: A
  - length_km: 0.1
  - note: мост на М3 к северу от Бухты

- id: brige_b_m2_05
  - from: brige_b_m2_05_1
  - to: brige_b_m2_05_2
  - class: B
  - length_km: 0.7
  - note: мост на М3 к северу от Бухты

- id: edge_b_m2_16
  - from: brige_b_m2_05_2
  - to: turbo_m1_m2_m1
  - class: B
  - length_km: 1
  - note: К развязке на трассе M1

## Е1
- id: edge_c_e1_01
  - from: t_cross_m2_e1
  - to: bridge_c_e1_01_01
  - class: C
  - length_km: 2
  - note: К развязке на трассе M1
  - obstacles:
      - id: roadwork_e1_03
        - type: roadwork
        - location: 0.40
        - side: right
      - road_blocked_e1_03
        - type: road_blocked
        - location: 0.75
        - side: left

- id: bridge_c_e1_01
  - from: bridge_c_e1_01_01
  - to: bridge_c_e1_01_02
  - class: C
  - length_km: 0.15
  - note: Мост на трассе Е1 к северо-западу от Южного

- id: edge_c_e1_02
  - from: bridge_c_e1_01_02
  - to: t-cross-m3-e1
  - class: C
  - length_km: 0.7
  - note: К развязке на трассе М3

- id: edge_c_e1_03
  - from: t-cross-m3-e1
  - to: bridge_c_e1_02_01
  - class: C
  - length_km: 4.9
  - note: К мосту на трассе Е1 к югу от Залесска

- id: bridge_c_e1_02
  - from: bridge_c_e1_02_01
  - to: bridge_c_e1_02_02
  - class: C
  - length_km: 0.15
  - note: Мост на трассе Е1 к Югу от Залесска

- id: edge_c_e1_04
  - from: bridge_c_e1_02_02
  - to: t-cross-e1-e1_01
  - class: C
  - length_km: 1.3
  - note: Поворот на Кольцо
  - poi:
    - gas_station_e1_01
      - name: gas_station_e1_01
      - type: gas_station
      - location: 0.9
      - side: right

- id: t-cross-e1-e1_01
  - from: t-cross-e1-e1_01
  - to: Zalessk_south_gate_01
  - class: C
  - length_km: 1.7
  - note: Южный въезд в Залессск
  - poi:
    - service_station_e1_01
      - name: service_station_e1_01
      - type: service_station
      - location: 0.3
      - side: right

- id: edge_c_e1_06
  - from: Zalessk_north_gate_01
  - to: bridge_c_e1_03_01
  - class: C
  - length_km: 4.5
  - note: Северный въезд в Залессск
  - poi:
      - police_e1_06
        - type: police_car
        - location: 0.35
        - side: right

- id: bridge_c_e1_03
  - from: bridge_c_e1_03_01
  - to: bridge_c_e1_03_02
  - class: C
  - length_km: 0.1
  - note: Мост к северу от Залесска

- id: edge_c_e1_07
  - from: bridge_c_e1_03_02
  - to: bridge_c_e1_04_01
  - class: C
  - length_km: 2.3
  - note: на 2-й мост к северу от Залесска

- id: bridge_c_e1_04
  - from: bridge_c_e1_04_01
  - to: bridge_c_e1_04_02
  - class: C
  - length_km: 0.3
  - note: 2-й мост к северу от Залесска

- id: edge_c_e1_08
  - from: bridge_c_e1_04_02
  - to: t-cross_e1_s3
  - class: C
  - length_km: 2.3
  - note: Секретная дорога на e3
  - poi:
    - gas_station_c_02
      - name: gas_station_c_02
      - type: gas_station
      - location: 0.9
      - side: right
  - obstacles:
      - roadwork_e1_08
        - type: roadwork
        - location: 0.50
        - side: left

- id: edge_c_e1_09
  - from: t-cross_e1_s3
  - to: bridge_c_e1_05_01
  - class: C
  - length_km: 4.3
  - note: к мосту к Югу от Ельнино
  - poi:
    - service_station_c_02
      - name: service_station_c_02
      - type: service_station
      - location: 0.5
      - side: right
    - police_e1_09
        - type: police_post
        - location: 0.25
        - side: right
  - obstacles:
      - road_blocked_e1_09
        - type: road_blocked
        - location: 0.70
        - side: left

- id: bridge_c_e1_05
  - from: bridge_c_e1_05_01
  - to: bridge_c_e1_05_02
  - class: C
  - length_km: 0.1
  - note: мост к Югу от Ельнино

- id: edge_c_e1_10
  - from: bridge_c_e1_05_02
  - to: Yelnino_south_gate_01
  - class: C
  - length_km: 2
  - note: мост к Югу от Ельнино

- id: edge_c_e1_11
  - from: Yelnino_north_gate_01
  - to: t-cross_e1_g1_01
  - class: C
  - length_km: 3
  - note: Поворот на ключи
  - poi:
    - service_station_e1_03
      - name: service_station_e1_03
      - type: service_station
      - location: 0.9
      - side: right
  - obstacles:
      - roadwork_e1_11
        - type: roadwork
        - location: 0.45
        - side: right
        
- id: edge_c_e1_12
  - from: t-cross_e1_g1_01
  - to: t-cross_e1_g1_02
  - class: C
  - length_km: 1
  - note: Поворот на ключи

- id: edge_c_e1_13
  - from: t-cross_e1_g1_02
  - to: bridge_c_e1_06_01
  - class: C
  - length_km: 0.7
  - note: На мост

- id: bridge_c_e1_06
  - from: bridge_c_e1_06_01
  - to: bridge_c_e1_06_02
  - class: C
  - length_km: 0.7
  - note: Мост

- id: edge_turquoise_e1_14
  - from: bridge_c_e1_06_02
  - to: turbo_m1_e1_m1
  - class: C
  - length_km: 1
  - note: На развязку с Основная трасса М1
  - poi:
    - gas_station_e1_04
      - name: gas_station_e1_04
      - type: gas_station
      - location: 0.4
      - side: left

## Е2
- id: edge_c_e2_01
  - from: turbo_m1_m2_m3
  - to: nizhnegorsk_north_gate_01
  - class: C
  - length_km: 1
  - note: въезд в Нижнегорск с Севера
  - poi:
    - gas_station_14
      - name: gas_station_14
      - type: gas_station
      - location: 0.1
      - side: left

- id: edge_c_e2_02
  - from: nizhnegorsk_south_gate_01
  - to: t-cross_e2_s1
  - class: C
  - length_km: 6
  - note: Секретная дорога на боровое
  - obstacles:
    - roadwork_e2_02
      - type: roadwork
      - location: 0.40
      - side: left
    - road_blocked_e2_02
      - type: road_blocked
      - location: 0.75
      - side: right

- id: edge_c_e2_03
  - from: t-cross_e2_s1
  - to: bridge_c_e2_01_01
  - class: C
  - length_km: 9
  - note: Секретная дорога на боровое
  - poi:
      - police_e2_03
        - type: police_car
        - location: 0.20
        - side: right
  - obstacles:
      - roadwork_e2_03_a
        - type: roadwork
        - location: 0.45
        - side: left
      - roadwork_e2_03_b
        - type: roadwork
        - location: 0.80
        - side: right 

- id: bridge_c_e2_01
  - from: bridge_c_e2_01_01
  - to: bridge_c_e2_01_02
  - class: C
  - length_km: 0.1
  - note: Первый мост в горах

- id: edge_c_e2_04
  - from: bridge_c_e2_01_02
  - to: bridge_c_e2_02_01
  - class: C
  - length_km: 2.1
  - note: Между 1 и 2 мостом

- id: bridge_c_e2_02
  - from: bridge_c_e2_02_01
  - to: bridge_c_e2_02_02
  - class: C
  - length_km: 0.1
  - note: Второй мост в горах

- id: edge_c_e2_05
  - from: bridge_c_e2_02_02
  - to: bridge_c_e2_03_01
  - class: C
  - length_km: 1
  - note: Третий мост в горах

- id: bridge_c_e2_03
  - from: bridge_c_e2_03_01
  - to: bridge_c_e2_03_02
  - class: C
  - length_km: 0.1
  - note: Третий мост в горах

- id: edge_c_e2_06
  - from: bridge_c_e2_03_02
  - to: gorny_entry_nw
  - class: C
  - length_km: 1.5
  - note: Въезд в Горный

- id: edge_c_e2_07
  - from: gorny_entry_se
  - to: t-cross_e2_s6
  - class: D
  - length_km: 3.8
  - note: от горного до блокпоста алмазный
  - obstacles:
      - road_blocked_e2_07
        - type: road_blocked
        - location: 0.50
        - side: left
        - note: частые перекрытия перед Алмазным

- id: edge_c_e2_08
  - from: t-cross_e2_s6
  - to: Almazniy_blockpost_01
  - class: D
  - length_km: 5
  - note: от горного до блокпоста алмазный
  - poi:
    - service_station_08
      - name: service_station_08
      - type: service_station
      - location: 0.1
      - side: right
    - police_e2_08
      - type: police_post
      - location: 0.30
      - side: right
      - note: блокпост перед Алмазным
## Е3
- id: edge_d_e3_01
  - from: Priozersk_north-west_gate_01
  - to: t-cross_e3_s3
  - class: D
  - length_km: 2
  - note: Секретная дорога на трассу e1

- id: edge_d_e3_02
  - from: t-cross_e3_s3
  - to: bridge_e3_01_01
  - class: D
  - length_km: 5
  - note: к мосту 1 на трассе приозерск-ельнино
  - obstacles:
      - id: roadwork_e3_02
      - type: roadwork
      - location: 0.40
      - side: left

- id: bridge_e3_01
  - from: bridge_e3_01_01
  - to: bridge_e3_01_02
  - class: D
  - length_km: 0.05
  - note: мост 1 на трассе приозерск-ельнино

- id: edge_purple_e3_03
  - from: bridge_e3_01_02
  - to: bridge_e3_02_01
  - class: D
  - length_km: 0.5
  - note: к мосту 2 на трассе приозерск-ельнино
  - obstacles:
      - id: road_blocked_e3_07
      - type: road_blocked
      - location: 0.60
      - side: right

- id: bridge_e3_02
  - from: bridge_e3_02_01
  - to: bridge_e3_02_02
  - class: D
  - length_km: 0.05
  - note: мост 2 на трассе приозерск-ельнино

- id: edge_d_e3_04
  - from: bridge_e3_02_02
  - to: t-cross_s4_e3
  - class: D
  - length_km: 0.5
  - note: секретная дорога на М2

- id: edge_d_e3_05
  - from: t-cross_s4_e3
  - to: t-cross_e1_s5
  - class: D
  - length_km: 0.5
  - note: секретная дорога на Ключи

- id: edge_d_e3_06
  - from: t-cross_e1_s5
  - to: bridge_d_e3_03_01
  - class: D
  - length_km: 0.5
  - note: к мосту 3 на трассе приозерск-ельнино

- id: bridge_d_e3_03
  - from: bridge_d_e3_03_01
  - to: bridge_d_e3_03_02
  - class: D
  - length_km: 0.05
  - note: мост 3 на трассе приозерск-ельнино

- id: edge_d_e3_07
  - from: bridge_d_e3_03_02
  - to: Yelnino_east_gate_01
  - class: D
  - length_km: 3
  - note: Восточный въезд в Ельнино

## E4

- id: edge_d_e4_01
  - from: intersection_m2_e4_01
  - to: Podgorsk_west_gate_01
  - class: D
  - length_km: 3
  - note: Западный въезд в Подгорск
  - poi: 
    - parking_e4_0`
      - name: parking_e4_0
      - type: parking
      - location: 0.95
      - side: left
    - service_station_e4_01
      - name: service_station_e4_01
      - type: service_station
      - location: 0.1
      - side: left
    - police_e4_01
        - type: police_car
        - location: 0.55
        - side: left

- id: edge_d_e4_02
  - from: Podgorsk_east_gate_01
  - to: t-cross_m3_e4
  - class: D
  - length_km: 3
  - note: Западный въезд в Подгорск
  - obstacles:
      - roadwork_e4_02
      - type: roadwork
      - location: 0.45
      - side: right

### G1

- id: edge_white_g1_01
  - from: t-cross_e1_g1_01
  - to: t-cross_01_white_G1_white_G1
  - class: D
  - length_km: 0.5
  - note: Поворот на ключи

- id: edge_white_g1_02
  - from: t-cross_01_white_G1_white_G1
  - to: t-cross_e1_g1_02
  - class: D
  - length_km: 0.5
  - note: Поворот на ключи

- id: edge_white_g1_03
  - from: t-cross_01_white_G1_white_G1
  - to: Kluchi_east_gate_01
  - class: D
  - length_km: 1.5
  - note: Поворот на ключи

### G2
- id: edge_white_g2_01
  - from: Almazniy_blockpost_01
  - to: t-cross_01_white_G2_yellow_s7
  - class: D
  - length_km: 0.7
  - note: грунтовая дорога на алмазный

- id: edge_white_g2_02
  - from: t-cross_01_white_G2_yellow_s7
  - to: t-cross_01_white_G2_white_G2
  - class: D
  - length_km: 0.7
  - note: грунтовая дорога на алмазный (пересечение со скрытой)

- id: edge_white_g2_03
  - from: t-cross_01_white_G2_white_G2
  - to: deadend_g2_01
  - class: D
  - length_km: 0.7
  - note: грунтовая дорога на алмазный (тупик)

- id: edge_white_g2_04
  - from: t-cross_01_white_G2_white_G2
  - to: t-cross_01_white_G2_yellow_s9_01
  - class: D
  - length_km: 1
  - note: грунтовая дорога на алмазный (пересечение со скрытой)

- id: edge_white_g2_05
  - from: t-cross_01_white_G2_yellow_s9_01
  - to: t-cross_01_white_G2_yellow_s9_02
  - class: D
  - length_km: 1
  - note: грунтовая дорога на алмазный (пересечение со скрытой)

- id: edge_white_g2_06
  - from: t-cross_01_white_G2_yellow_s9_02
  - to: t-cross_01_white_G2_yellow_s10
  - class: D
  - length_km: 2
  - note: грунтовая дорога на алмазный (пересечение со скрытой)

- id: edge_white_g2_07
  - from: t-cross_01_white_G2_yellow_s10
  - to: t-cross_01_white_G2_yellow_s08
  - class: D
  - length_km: 2
  - note: грунтовая дорога на алмазный (пересечение со скрытой)

- id: edge_white_g2_08
  - from: t-cross_01_white_G2_yellow_s08
  - to: t-cross_01_white_G2_yellow_s11
  - class: D
  - length_km: 0.2
  - note: грунтовая дорога на алмазный (пересечение со скрытой)

- id: edge_white_g2_09
  - from: t-cross_01_white_G2_yellow_s11
  - to: t-cross_02_white_G2_yellow_s7
  - class: D
  - length_km: 0.7
  - note: грунтовая дорога на алмазный (пересечение со скрытой)

- id: edge_white_g2_10
  - from: t-cross_02_white_G2_yellow_s7
  - to: t-cross_01_white_G2_yellow_s12
  - class: D
  - length_km: 4.5
  - note: грунтовая дорога на алмазный (пересечение со скрытой)

- id: edge_white_g2_11
  - from: t-cross_01_white_G2_yellow_s12
  - to: x-cross_01_white_G2_yellow_s12
  - class: D
  - length_km: 0.4
  - note: грунтовая дорога на алмазный (пересечение со скрытой)

- id: edge_white_g2_12
  - from: x-cross_01_white_G2_yellow_s12
  - to: t-cross_01_white_G2_yellow_s13_01
  - class: D
  - length_km: 0.2
  - note: грунтовая дорога на алмазный (пересечение со скрытой)

- id: edge_white_g2_13
  - from: t-cross_01_white_G2_yellow_s13_01
  - to: t-cross_01_white_G2_yellow_s13_02
  - class: D
  - length_km: 1.5
  - note: грунтовая дорога на алмазный (пересечение со скрытой)

- id: edge_white_g2_14
  - from: t-cross_01_white_G2_yellow_s13_02
  - to: Almazniy_north_gate_01
  - class: D
  - length_km: 3
  - note: Северный въезд в Алмазный



### S1
- id: edge_yellow_s1
  - from: t-cross_m3_s1
  - to: t-cross_e2_s1
  - class: S
  - length_km: 1.5
  - note: Скрытая дорога между Боровое и Горный-Нижнегорск

### S2
- id: edge_yellow_s2
  - from: t-cross_m3_s2
  - to: t-cross_s2_s10
  - class: S
  - length_km: 10
  - note: Скрытая дорога в горы

### S3
- id: edge_s3
  - from: t-cross_e1_s3
  - to: t-cross_e3_s3
  - class: S
  - length_km: 4
  - note: Секретная между e1 и е3

### S4
- id: edge_yellow_s3
  - from: t_cross_m2_s4
  - to: t-cross_s4_e3
  - class: S
  - length_km: 4
  - note: Секретная между m2 и е3

### S5
- id: edge_yellow_s5
  - from: t-cross_e1_s5
  - to: Kluchi_south_gate_01
  - class: S
  - length_km: 4
  - note: Секретная между e3 и Ключами

### S6
- id: edge_yellow_s6
  - from: t-cross_e2_s6
  - to: t-cross_yellow_s7_yellow_s6
  - class: S
  - length_km: 4
  - note: Секретная дорога в горах

### S7
- id: edge_yellow_s7_01
  - from: t-cross_01_white_G2_yellow_s7
  - to: t-cross_yellow_s7_yellow_s6
  - class: S
  - length_km: 4
  - note: Секретная дорога в горах

- id: edge_yellow_s7_02
  - from: t-cross_yellow_s7_yellow_s6
  - to: t-cross_02_white_G2_yellow_s7
  - class: S
  - length_km: 4
  - note: Секретная дорога в горах

### S8
- id: edge_yellow_s8_01
  - from: t-cross_yellow_s9_yellow_s8
  - to: t-cross_01_white_G2_yellow_s08
  - class: S
  - length_km: 1
  - note: Секретная дорога в горах

### S9
- id: edge_yellow_s9_01
  - from: t-cross_01_white_G2_yellow_s9_01
  - to: t-cross_yellow_s9_yellow_s8
  - class: S
  - length_km: 0.5
  - note: Секретная дорога в горах

- id: edge_yellow_s9_02
  - from: t-cross_yellow_s9_yellow_s8
  - to: t-cross_01_white_G2_yellow_s9_02
  - class: S
  - length_km: 0.5
  - note: Секретная дорога в горах

### S10
- id: edge_yellow_s10_01
  - from: t-cross_01_white_G2_yellow_s10
  - to: t-cross_s2_s10
  - class: S
  - length_km: 0.5
  - note: Секретная дорога в горах

- id: edge_yellow_s10_02
  - from: t-cross_s2_s10
  - to: t-cross_s10_yellow_s11_yellow
  - class: S
  - length_km: 1
  - note: Секретная дорога в горах

- id: edge_yellow_s10_03
  - from: t-cross_s10_yellow_s11_yellow
  - to: t-cross_s10_yellow_s12_yellow
  - class: S
  - length_km: 2
  - note: Секретная дорога в горах

- id: edge_yellow_s10_04
  - from: t-cross_s10_yellow_s12_yellow
  - to: Almazniy_west_gate_01
  - class: S
  - length_km: 4
  - note: Секретная дорога в горах

### S11
- id: edge_yellow_s11_01
  - from: t-cross_s10_yellow_s11_yellow
  - to: t-cross_01_white_G2_yellow_s11
  - class: S
  - length_km: 0.5
  - note: Секретная дорога в горах

### S12
- id: edge_yellow_s12_01
  - from: t-cross_s10_yellow_s12_yellow
  - to: x-cross_01_white_G2_yellow_s12
  - class: S
  - length_km: 0.7
  - note: Секретная дорога в горах

- id: edge_yellow_s12_02
  - from: x-cross_01_white_G2_yellow_s12
  - to: t-cross_01_white_G2_yellow_s12
  - class: S
  - length_km: 0.4
  - note: Секретная дорога в горах

### S13
- id: edge_yellow_s13_01
  - from: t-cross_01_white_G2_yellow_s13_01
  - to: t-cross_01_white_G2_yellow_s13_02
  - class: S
  - length_km: 1.2
  - note: Секретная дорога в горах


## Города

### Южный
#### Дороги
- id: Yuzhniy_bypass_01
  - from: west_yuzhniy_gate_01
  - to: east_yuzhniy_gate_01
  - class: B
  - length_km: 1.2
  - note: Западный въезд в южный
  - poi:
    - gas_station_yuzhniy_01
      - name: gas_station_yuzhniy_01
      - type: gas_station
      - location: 0.1
      - side: right
    - gas_station_yuzhniy_02
      - name: gas_station_yuzhniy_02
      - type: gas_station
      - location: 0.55
      - side: left
    - warehouse_yuzhniy_01
      - name: warehouse_yuzhniy_01
      - type: warehouse
      - location: 0.35
      - side: left
    - service_station_yuzhniy_01
      - name: service_station_yuzhniy_01
      - type: service_station
      - location: 0.2
      - side: left
    - parking_yuzhniy_01
      - name: parking_yuzhniy_01
      - type: parking
      - location: 0.95
      - side: left

### Залесск
#### Дороги
- id: Zalessk_bypass_01
  - from: Zalessk_south_gate_01
  - to: Zalessk_north_gate_01
  - class: B
  - length_km: 0.570
  - note: Сквозной проезд через залесск
  - poi:
    - gas_station_102
      - name: gas_station_102
      - type: gas_station
      - location: 0.63
      - side: left
    - warehouse_02
      - name: warehouse_zalessk_01
      - type: warehouse
      - location: 0.15
      - side: right
    - parking_102
      - name: parking_102
      - type: parking
      - location: 0.95
      - side: right

### Ельнино
#### Дороги
- id: Elnino_bypass_01
  - from: Yelnino_south_gate_01
  - to: t-cross-Elnino_bypass_01-Elnino_street01_01
  - class: B
  - length_km: 0.01
  - note: прекресток в Ельнино с первой улицей

- id: Elnino_bypass_02
  - from: t-cross-Elnino_bypass_01-Elnino_street01_01
  - to: t-cross-Elnino_bypass_01-Elnino_street02_01
  - class: B
  - length_km: 0.750
  - note: прекресток в Ельнино со второй улицей
  - poi:
      - warehouse_03
      - name: warehouse_elnino_01
      - type: warehouse
      - location: 1
      - side: left

- id: Elnino_bypass_03
  - from: t-cross-Elnino_bypass_01-Elnino_street02_01
  - to: t-cross-Elnino_bypass_01-Elnino_street03_01
  - class: B
  - note: прекресток в Ельнино с третий улицей

- id: Elnino_bypass_04
  - from: t-cross-Elnino_bypass_01-Elnino_street03_01
  - to: Yelnino_north_gate_01
  - class: B
  - length_km: 1.250
  - note: прекресток в Ельнино с третий улицей
  - poi:
    - gas_station_102
      - name: gas_station_102
      - type: gas_station
      - location: 0.63
      - side: left
      
- id: Yelnino_steet01_01
  - from: t-cross-Elnino_bypass_01-Elnino_street01_01
  - to: t-cross-Elnino_street01_01-Elnino_street04_01
  - class: D
  - length_km: 0.650
  - note: Перекресток в Ельнино 1 и 4 улица
  - poi: 
      - service_station_102
      - name: service_station_102
      - type: service_station
      - location: 0.65
      - side: left

- id: Yelnino_steet01_02
  - from: t-cross-Elnino_street01_01-Elnino_street04_01
  - to: Yelnino_east_gate_01
  - class: D
  - length_km: 0.650
  - note: Перекресток в Ельнино 1 и 4 улица, выезд из Ельнино

- id: Yelnino_steet04_01
  - from: t-cross-Elnino_street01_01-Elnino_street04_01
  - to: t-cross-Elnino_street04_01_Elnino_street02_01_Elnino_street03_01
  - class: D
  - length_km: 0.750
  - note: Перекресток в Ельнино 4 2 и 3 улица

- id: Yelnino_steet02_01
  - from: t-cross-Elnino_bypass_01-Elnino_street02_01
  - to: t-cross-Elnino_street04_01_Elnino_street02_01_Elnino_street03_01
  - class: D
  - length_km: 0.650
  - note: Перекресток в Ельнино 4 2 и 3 улица

- id: Yelnino_steet03_01
  - from: t-cross-Elnino_bypass_01-Elnino_street03_01
  - to: t-cross-Elnino_street04_01_Elnino_street02_01_Elnino_street03_01
  - class: D
  - length_km: 1
  - note: Перекресток в Ельнино 4 2 и 3 улица
  - poi:
      - parking_103
      - name: parking_103
      - type: parking
      - location: 0.95
      - side: left

### Ключи
#### Дороги
- id: Kluchi_bypass_01
  - from: Kluchi_east_gate_01
  - to: connection_Kluchi_white_G1_yellow_S5
  - class: D
  - length_km: 0.570
  - note: Въезд в Ключи с запада
  - poi:
    - warehouse_04
      - name: warehouse_kluchi_01
      - type: warehouse
      - location: 0.5
      - side: left

- id: Kluchi_s5_01
  - from: connection_Kluchi_white_G1_yellow_S5
  - to: Kluchi_south_gate_01
  - class: S
  - length_km: 0.270
  - note: Секретная между e3 и Ключами

### Бухта
#### Дороги
- id: Buhta_road_01
  - from: buhta_south_gate_01
  - to: roundbound_01_buhta_ringroad_01_ringroad_03_Buhta_road_01
  - class: B
  - length_km: 0.15
  - note: К окружной Бухты

- id: buhta_ringroad_01
  - from: roundbound_01_buhta_ringroad_01_ringroad_03_Buhta_road_01
  - to: t-cross_buhta_ringroad_01_buhta_ringroad_02_buhta_road_02
  - class: B
  - length_km: 0.370
  - note: 1 часток окружной Бухты

- id: buhta_ringroad_02
  - from: t-cross_buhta_ringroad_01_buhta_ringroad_02_buhta_road_02
  - to: x-cross_buhta_ringroad_02_buhta_ringroad_03_buhta_road_02_Buhta_road_03
  - class: B
  - length_km: 2
  - note: 2 часток окружной Бухты
  - poi:
    - warehouse_05
      - name: warehouse_buhta_01
      - type: warehouse
      - location: 0.3
      - side: left
    - service_station_103
      - name: service_station_buhta_01
      - type: service_station
      - location: 0.5
      - side: right

- id: buhta_ringroad_03
  - from: x-cross_buhta_ringroad_02_buhta_ringroad_03_buhta_road_02_Buhta_road_03
  - to: roundbound_01_buhta_ringroad_01_ringroad_03_Buhta_road_01
  - class: B
  - length_km: 0.470
  - note: 3 часток окружной Бухты

- id: Buhta_road_02
  - from: x-cross_buhta_ringroad_02_buhta_ringroad_03_buhta_road_02_Buhta_road_03
  - to: t-cross_buhta_ringroad_01_buhta_ringroad_02_buhta_road_02
  - class: C
  - length_km: 0.6
  - note: Сквозной проезд через Бухту
  - poi:
      - parking_104
        - name: parking_104
        - type: parking
        - location: 0.5
        - side: left
      - gas_station_103
        - name: gas_station_103
        - type: gas_station
        - location: 0.5
        - side: right

- id: Buhta_road_03
  - from: x-cross_buhta_ringroad_02_buhta_ringroad_03_buhta_road_02_Buhta_road_03
  - to: buhta_north_gate_01
  - class: B
  - length_km: 0.05
  - note: Сквозной проезд через Бухту

### Приозерск
#### Дороги
- id: Priozersk_road_01
  - from: Priozersk_south_gate_01
  - to: x-cross_priozersk_ringroad_01_priozersk_ringroad_05_priozersk_road_04_Priozersk_road_01
  - class: B
  - length_km: 0.15
  - note: Въезд в Приозерск с юга
  - poi:
      - parking_priozersk_01
        - name: parking_priozersk_01
        - type: parking
        - location: 0.5
        - side: left
      - service_station_priozersk_01
        - name: service_station_priozersk_01
        - type: service_station
        - location: 0.5
        - side: right
  
- id: priozersk_ringroad_01
  - from: x-cross_priozersk_ringroad_01_priozersk_ringroad_05_priozersk_road_04_Priozersk_road_01
  - to: t-cross_ringroad_01_ringroad_02_road_05_01
  - class: B
  - length_km: 0.6
  - note: Въезд в Приозерск с юга
  - poi:
    - warehouse_06
      - name: warehouse_priozersk_01
      - type: warehouse
      - location: 0.3
      - side: left

- id: priozersk_ringroad_02
  - from: t-cross_ringroad_01_ringroad_02_road_05_01
  - to: t-cross_ringroad_02_ringroad_03_road_02
  - class: B
  - length_km: 0.25
  - note: Въезд в Приозерск с северо-запада
  - poi:
    - parking_106
      - name: parking_106
      - type: parking
      - location: 0.5
      - side: left

- id: priozersk_ringroad_03
  - from: t-cross_ringroad_02_ringroad_03_road_02
  - to: roundbound_priozersk_ringroad_03_priozersk_ringroad_04_priozersk_road_03_Priozersk_road_06
  - class: B
  - length_km: 0.3
  - note: Въезд в Приозерск с северо-востока

- id: priozersk_ringroad_04
  - from: roundbound_priozersk_ringroad_03_priozersk_ringroad_04_priozersk_road_03_Priozersk_road_06
  - to: t-cross_ringroad_04_ringroad_05_road_06
  - class: B
  - length_km: 0.3
  - note: 

- id: priozersk_ringroad_05
  - from: t-cross_ringroad_04_ringroad_05_road_06
  - to: x-cross_priozersk_ringroad_01_priozersk_ringroad_05_priozersk_road_04_Priozersk_road_01
  - class: B
  - length_km: 0.3
  - note: 
  - poi:
      - gas_station_104
        - name: gas_station_104
        - type: gas_station
        - location: 0.1
        - side: left

- id: priozersk_road_02
  - from: t-cross_ringroad_02_ringroad_03_road_02
  - to: Priozersk_north-west_gate_01
  - class: B
  - length_km: 0.4
  - note: 

- id: priozersk_road_03
  - from: roundbound_priozersk_ringroad_03_priozersk_ringroad_04_priozersk_road_03_Priozersk_road_06
  - to: Priozersk_north-east_gate_01
  - class: B
  - length_km: 0.4
  - note: 

- id: priozersk_road_04
  - from: x-cross_priozersk_ringroad_01_priozersk_ringroad_05_priozersk_road_04_Priozersk_road_01
  - to: t-cross-Priozersk_road_04-Priozersk_road_05
  - class: B
  - length_km: 0.4
  - note: 

- id: priozersk_road_05_01
  - from: t-cross_ringroad_01_ringroad_02_road_05_01
  - to: t-cross-Priozersk_road_04-Priozersk_road_05
  - class: B
  - length_km: 0.4
  - note: 

- id: priozersk_road_05_02
  - from: t-cross-Priozersk_road_04-Priozersk_road_05
  - to: t-cross-Priozersk_road_04-Priozersk_road_06
  - class: B
  - length_km: 0.4
  - note: 

- id: priozersk_road_06_01
  - from: roundbound_priozersk_ringroad_03_priozersk_ringroad_04_priozersk_road_03_Priozersk_road_06
  - to: t-cross-Priozersk_road_04-Priozersk_road_06
  - class: B
  - length_km: 0.4
  - note: 

- id: priozersk_road_06_02
  - from: t-cross-Priozersk_road_04-Priozersk_road_06
  - to: t-cross_ringroad_04_ringroad_05_road_06
  - class: B
  - length_km: 0.4
  - note: 

### Подгорск
#### Дороги
- id: Podgorsk_road_01
  - from: Podgorsk_west_gate_01
  - to: t-cross_Podgorsk_ringroad_01_ringroad_04_Podgorsk_road_01
  - class: B
  - length_km: 0.1
  - note: Сквозной проезд через Подгорск

- id: Podgorsk_ringroad_01
  - from: t-cross_Podgorsk_ringroad_01_ringroad_04_Podgorsk_road_01
  - to: t-cross_Podgorsk_ringroad_01_Podgorsk_ringroad_02_Podgorsk_road_02
  - class: B
  - length_km: 0.670
  - note: 1 часток окружной Подгорск
  - poi:
    - warehouse_Podgorsk_01
      - name: warehouse_Podgorsk_01
      - type: warehouse
      - location: 0.4
      - side: left
    - parking_107
      - name: parking_Podgorsk_01
      - type: parking
      - location: 0.5
      - side: right

- id: Podgorsk_ringroad_02
  - from: t-cross_Podgorsk_ringroad_01_Podgorsk_ringroad_02_Podgorsk_road_02
  - to: t-cross_Podgorsk_ringroad_02_ringroad_03_Podgorsk_road_03
  - class: B
  - length_km: 0.670
  - note: 2 часток окружной Подгорск

- id: Podgorsk_ringroad_03
  - from: t-cross_Podgorsk_ringroad_02_ringroad_03_Podgorsk_road_03
  - to: t-cross_Podgorsk_ringroad_03_Podgorsk_ringroad_04_Podgorsk_road_02
  - class: B
  - length_km: 0.470
  - note: 3 часток окружной Подгорск
  - poi:
      - gas_station_Podgorsk_01
        - name: gas__Podgorsk_01
        - type: gas_station
        - location: 0.5
        - side: left

- id: Podgorsk_ringroad_04
  - from: t-cross_Podgorsk_ringroad_03_Podgorsk_ringroad_04_Podgorsk_road_02
  - to: t-cross_Podgorsk_ringroad_01_ringroad_04_Podgorsk_road_01
  - class: B
  - length_km: 0.670
  - note: 2 часток окружной Подгорск

- id: Podgorsk_road_02
  - from: t-cross_Podgorsk_ringroad_03_Podgorsk_ringroad_04_Podgorsk_road_02
  - to: t-cross_Podgorsk_ringroad_01_Podgorsk_ringroad_02_Podgorsk_road_02
  - class: C
  - length_km: 0.6
  - note: Сквозной проезд через Подгорск

- id: Podgorsk_road_03
  - from: t-cross_Podgorsk_ringroad_02_ringroad_03_Podgorsk_road_03
  - to: Podgorsk_east_gate_01
  - class: B
  - length_km: 0.05
  - note: Сквозной проезд через Подгорск

### Шахты
#### Дороги
- id: Shahty_bypass_01
  - from: shahty_west_gate_1
  - to: t-cross_Shahty_bypass_01_Shahty_bypass_02_Shahty_road_01
  - class: C
  - length_km: 0.5
  - note: Сквозной проезд через Шахты
  - poi:
      - gas_station_105
        - name: gas_station_105
        - type: gas_station
        - location: 0.1
        - side: right

- id: Shahty_bypass_02
  - from: t-cross_Shahty_bypass_01_Shahty_bypass_02_Shahty_road_01
  - to: shahty_east_gate_1
  - class: C
  - length_km: 0.5
  - note: Сквозной проезд через Шахты
  - poi:
      - gas_station_105
        - name: gas_station_105
        - type: gas_station
        - location: 0.1
        - side: right
      - service_station_105
        - name: service_station_105
        - type: service_station
        - location: 0.1
        - side: left

- id: Shahty_road_01
  - from: t-cross_Shahty_bypass_01_Shahty_bypass_02_Shahty_road_01
  - to: Shahty_road_01_deadend
  - class: C
  - length_km: 0.5
  - note: Сквозной проезд через Подгорск
  - poi:
    - warehouse_08
      - name: warehouse_shahty_01
      - type: warehouse
      - location: 0.1
      - side: left

### Боровое
#### Дороги
- id: borovoe_bypass_01
  - from: borovoe_gate_1_west
  - to: t-cross_Borovoe_bypass_borovoe_road_01
  - class: C
  - length_km: 0.4
  - note: Главная улица
  - poi:
    - gas_station_borovoe_01
      - name: gas_station_borovoe_01
      - type: gas_station
      - location: 0.15
      - side: left

- id: borovoe_bypass_02
  - from: t-cross_Borovoe_bypass_borovoe_road_01
  - to: t-cross_Borovoe_bypass_borovoe_road_02
  - class: C
  - length_km: 0.4
  - note: Главная улица

- id: borovoe_bypass_03
  - from: t-cross_Borovoe_bypass_borovoe_road_02
  - to: t-cross_Borovoe_bypass_borovoe_road_03
  - class: C
  - length_km: 0.4
  - note: Главная улица

- id: borovoe_bypass_04
  - from: t-cross_Borovoe_bypass_borovoe_road_03
  - to: t-cross_Borovoe_bypass_borovoe_road_04
  - class: C
  - length_km: 0.4
  - note: Главная улица
  - poi:
    - warehouse_borovoe_01
      - name: warehouse_borovoe_01
      - type: warehouse
      - location: 0.35
      - side: right
    - service_station_borovoe_01
      - name: service_station_borovoe_01
      - type: service_station
      - location: 0.75
      - side: right
      
- id: borovoe_bypass_05
  - from: t-cross_Borovoe_bypass_borovoe_road_04
  - to: borovoe_gate_1_east
  - class: C
  - length_km: 0.4
  - note: Главная улица

- id: borovoe_road_01
  - from: t-cross_Borovoe_bypass_borovoe_road_01
  - to: t-cross_Borovoe_road_01_borovoe_road_05_01
  - class: C
  - length_km: 1
  - note: Первая улица

- id: borovoe_road_02
  - from: t-cross_Borovoe_bypass_borovoe_road_02
  - to: t-cross_Borovoe_road_02_borovoe_road_05_02
  - class: C
  - length_km: 1
  - note: Вторая улица

- id: borovoe_road_3_01
  - from: t-cross_Borovoe_bypass_borovoe_road_03
  - to: t-cross_Borovoe_road_03_borovoe_road_05_03
  - class: C
  - length_km: 1
  - note: Третья улица

- id: borovoe_road_3_02
  - from: t-cross_Borovoe_road_03_borovoe_road_05_03
  - to: t-cross_Borovoe_road_03_borovoe_road_04
  - class: C
  - length_km: 1
  - note: Третья улица

- id: borovoe_road_05_01
  - from: t-cross_Borovoe_road_01_borovoe_road_05_01
  - to: t-cross_Borovoe_road_02_borovoe_road_05_02
  - class: C
  - length_km: 0.4
  - note: Пятая улица

- id: borovoe_road_05_03
  - from: t-cross_Borovoe_road_02_borovoe_road_05_02
  - to: t-cross_Borovoe_road_03_borovoe_road_05_03
  - class: C
  - length_km: 0.4
  - note: Пятая улица

- id: borovoe_road_04
  - from: t-cross_Borovoe_road_03_borovoe_road_04
  - to: t-cross_Borovoe_bypass_borovoe_road_04
  - class: C
  - length_km: 0.4
  - note: Пятая улица


### Нижнегорск
#### Дороги

- id: nizhnegorsk_bypass_01
  - from: nizhnegorsk_north_gate_01
  - to: t-cross-nizhnegorsk_bypass_nizhnegorsk_road_01_01
  - class: C
  - length_km: 0.4
  - note: Главная улица, северо-западный участок
  - poi:
    - service_station_nizhnegorsk_01
      - name: service_station_nizhnegorsk_01
      - type: service_station
      - location: 0.9
      - side: right

- id: nizhnegorsk_bypass_02
  - from: t-cross-nizhnegorsk_bypass_nizhnegorsk_road_01_01
  - to: t-cross-nizhnegorsk_bypass_nizhnegorsk_road_01_02
  - class: C
  - length_km: 0.4
  - note: Главная улица, северо-западный участок

- id: nizhnegorsk_bypass_03
  - from: t-cross-nizhnegorsk_bypass_nizhnegorsk_road_01_02
  - to: nizhnegorsk_south_gate_01
  - class: C
  - length_km: 0.4
  - note: Главная улица, северо-западный участок
  - poi:
      - gas_station_nizhnegorsk_01
        - name: gas_station_nizhnegorsk_01
        - type: gas_station
        - location: 1.0
        - side: right

- id: nizhnegorsk_road_01
  - from: t-cross-nizhnegorsk_bypass_nizhnegorsk_road_01_01
  - to: t-cross-nizhnegorsk_bypass_nizhnegorsk_road_01_02
  - class: C
  - length_km: 0.4
  - note: Юго-восточный участок
  - poi:
      - parking_nizhnegorsk_01
        - name: parking_nizhnegorsk_01
        - type: parking
        - location: 0.25
        - side: right
      - warehouse_nizhnegorsk_01
        - name: warehouse_nizhnegorsk_01
        - type: warehouse
        - location: 0.75
        - side: right

### Горный
#### Дороги

- id: gorny_bypass_01
  - from: gorny_entry_nw
  - to: t-cross_gorny_bypass_gorniy_road_01_1
  - class: C
  - length_km: 0.6
  - note: Сквозной проезд

- id: gorny_bypass_02
  - from: t-cross_gorny_bypass_gorniy_road_01_1
  - to: t-cross_gorny_bypass_gorniy_road_02_1
  - class: C
  - length_km: 1.3
  - note: Сквозной проезд

- id: gorny_bypass_03
  - from: t-cross_gorny_bypass_gorniy_road_02_1
  - to: gorny_entry_se
  - class: C
  - length_km: 0.2
  - note: Сквозной проезд

- id: gorny_road_01_1
  - from: t-cross_gorny_bypass_gorniy_road_01_1
  - to: x-cross-gorniy_road_01_gorniy_road_02
  - class: C
  - length_km: 0.7
  - note: Юго-восточный участок
  - poi:
      - parking_gorny_01
        - name: parking_gorny_01
        - type: parking
        - location: 0.25
        - side: right
      - warehouse_gorny_01
        - name: warehouse_gorny_01
        - type: warehouse
        - location: 0.75
        - side: right

  - poi:
      - parking_gorny_01
        - name: parking_gorny_01
        - type: parking
        - location: 1.0
        - side: right

  - poi:
      - warehouse_gorny_01
        - name: warehouse_gorny_01
        - type: warehouse
        - location: 1.0
        - side: right

  - poi:
      - service_station_gorny_01
        - name: service_station_gorny_01
        - type: service_station
        - location: 1.0
        - side: right

  - poi:
      - gas_station_gorny_01
        - name: gas_station_gorny_01
        - type: gas_station
        - location: 1.0
        - side: right

### Алмазный
#### Дороги
- id: Almazniy_north_gate_01
  - from: Almazniy_north_gate_01
  - to: connection_Almazniy_white_g2_13_yellow_S10
  - class: D
  - length_km: 0.570
  - note: Въезд в Алмазный с севера
  - poi:
    - warehouse_12
      - name: warehouse_almazniy_01
      - type: warehouse
      - location: 0.5
      - side: right

- id: Almazniy_s10_01
  - from: Almazniy_west_gate_01
  - to: connection_Almazniy_white_g2_13_yellow_S10
  - class: S
  - length_km: 0.270
  - note: Секретная дорога в горах

## Развязки
turbo_m1_e1_m1
  - id: ramp_t_e1_m1_01
    - from: edge_turquoise_e1_14
      - position: 0.95
      - side: right
    - to: edge_a_m1_02
      - position: 0.05
      - side: right
    - obstacles:
      - roadwork
        - id: roadwork_ramp_t_m1_01
        - position: 0.5
    - class: ramp
    - length_km: 0.300

  - id: ramp_t_m1_e1_02
    - from: edge_a_m1_01
      - position: 0.05
    - to: edge_turquoise_e1_14
      - position: 0.90
    - class: ramp
    - length_km: 0.300

turbo_m1_m2_m1
  - id: ramp_t_m2_m1_01
    - from: edge_b_m2_16
      - position: 0.95
      - side: right
    - to: edge_a_m1_03
      - position: 0.05
      - side: right
    - class: ramp
    - length_km: 0.300

  - id: ramp_t_m1_m2_02
    - from: edge_a_m1_01
      - position: 0.05
      - side: left
    - to: ramp_t_m1_m2_01
      - side: left
    - class: ramp
    - length_km: 0.300

  - id: ramp_t_m1_m2_03_01
    - from: edge_a_m1_02
      - position: 0.90
      - side: right
    - to: ramp_t_m1_m2_01
      - side: right
    - class: ramp
    - length_km: 0.300
  - id: ramp_t_m1_m2_03_02
    - from: ramp_t_m1_m2_01
      - position: 0.90
      - side: right
    - to: edge_b_m2_16
      - position: 0.70
      - side: left
    - class: ramp
    - length_km: 0.500

  - id: ramp_t_m1_m2_04
    - from: edge_b_m2_16
      - position: 1.00
      - side: right
    - to: edge_a_m1_02
      - position: 0.950
      - side: left
    - class: ramp
    - length_km: 0.800

turbo_m1_m1_m1
  - id: ramp_t_m1_m1_01_01
    - from: edge_a_m1_05
      - position: 0.90
      - side: right
    - to: ramp_t_m1_m1_02
      - side: left
    - class: ramp
    - length_km: 1.00
  - id: ramp_t_m1_m1_01_02
    - from: ramp_t_m1_m1_02
      - position: 0.90
      - side: right
    - to: edge_a_m1_07
      - side: right
    - class: ramp
    - length_km: 0.15

  - id: ramp_t_m1_m1_01_02
    - from: edge_a_m1_07
      - position: 0.10
      - side: left
    - to: edge_a_m1_06
      - position: 0.15
      - side: right
    - class: ramp
    - length_km: 1.20

  - id: ramp_t_m1_m1_01_03
    - from: edge_a_m1_06
      - position: 0.10
      - side: right
    - to: ramp_t_m1_m1_02
      - position: 0.15
      - side: right
    - class: ramp
    - length_km: 0.35

turbo_m1_e2_m3
  - id: ramp_t_e2_m1_01
    - from: edge_c_e2_01
      - position: 0.10
      - side: left
    - to: edge_a_m1_06
      - position: 0.90
      - side: right
    - class: ramp
    - length_km: 0.3

  - id: ramp_t_m3_e2_02
    - from: edge_b_m3_01
      - position: 0.10
      - side: left
    - to: edge_c_e2_01
      - position: 0.10
      - side: right
    - class: ramp
    - length_km: 0.4

  - id: ramp_t_m3_e2_03
    - from: edge_b_m3_01
      - position: 0.01
      - side: left
    - to: edge_c_e2_01
      - position: 0.00
      - side: right
    - class: ramp
    - length_km: 0.4

  - id: ramp_t_m3_e2_04
    - from: edge_c_e2_01
      - position: 0.00
      - side: left
    - to: edge_b_m3_01
      - position: 0.50
      - side: right
    - class: ramp
    - length_km: 0.5

turbo_m2_e4_m2
  - id: ramp_t_e4_m2_01
    - from: edge_d_e4_01
      - position: 0.10
      - side: left
    - to: edge_b_m2_13
      - position: 0.10
      - side: right
    - class: ramp
    - length_km: 0.3

  - id: ramp_t_e4_m2_02
    - from: edge_b_m2_13
      - position: 0.90
      - side: left
    - to: edge_d_e4_01
      - position: 0.10
      - side: right
    - class: ramp
    - length_km: 0.4

  - id: ramp_t_e4_m2_03
    - from: edge_b_m2_13
      - position: 0.17
      - side: left
    - to: edge_d_e4_01
      - position: 0.00
      - side: right
    - class: ramp
    - length_km: 0.4

  - id: ramp_t_e4_m2_04
    - from: edge_d_e4_01
      - position: 0.00
      - side: left
    - to: edge_b_m2_13
      - position: 0.1
      - side: left
    - class: ramp
    - length_km: 0.5
