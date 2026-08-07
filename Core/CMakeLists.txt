# --- Библиотека ядра ---
add_library(KotRCore STATIC
    src/core/TimeSystem.cpp
    src/economy/EconomySystem.cpp
    src/data/DataLoader.cpp
    src/contracts/ContractSystem.cpp
    src/finance/FinanceSystem.cpp
    src/market/MarketSystem.cpp
    src/drivers/DriverSystem.cpp
    src/drivers/DriverAgentSystem.cpp
    src/vehicles/VehicleSystem.cpp
    src/leaderboard/DeliveryLeaderboard.cpp
    src/licenses/LicenseSystem.cpp
     src/risk/RiskSystem.cpp
    src/radio/RadioSystem.cpp 
    tests/test_risk.cpp
)

target_include_directories(KotRCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

# --- Тесты ---
add_executable(KotRTests tests/test_main.cpp)
target_link_libraries(KotRTests PRIVATE KotRCore)

add_executable(KotRTestEconomy tests/test_economy.cpp)
target_link_libraries(KotRTestEconomy PRIVATE KotRCore)

add_executable(KotRTestDataLoader tests/test_data_loader.cpp)
target_link_libraries(KotRTestDataLoader PRIVATE KotRCore)

add_executable(KotRTestContracts tests/test_contract.cpp)
target_link_libraries(KotRTestContracts PRIVATE KotRCore)

add_executable(KotRTestMarket tests/test_market.cpp)
target_link_libraries(KotRTestMarket PRIVATE KotRCore)

add_executable(KotRTestDrivers tests/test_drivers.cpp)
target_link_libraries(KotRTestDrivers PRIVATE KotRCore)

add_executable(KotRTestDriverAgents tests/test_driver_agents.cpp)
target_link_libraries(KotRTestDriverAgents PRIVATE KotRCore)

add_executable(KotRTestVehicles tests/test_vehicles.cpp)
target_link_libraries(KotRTestVehicles PRIVATE KotRCore)

add_executable(KotRTestLeaderboard tests/test_leaderboard.cpp)
target_link_libraries(KotRTestLeaderboard PRIVATE KotRCore)

add_executable(KotRTestRisk tests/test_risk.cpp)
target_link_libraries(KotRTestRisk PRIVATE KotRCore)

# Копируем data/ в build для тестов
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})