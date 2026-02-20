cmake_minimum_required(VERSION 3.20)
project(ryans-realtime-audio-router)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_PREFIX_PATH "/usr/local")

# ===== Core library =====
add_library(core
    src/AudioEngine.cpp
    src/SimDriver.cpp
)

target_include_directories(core PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

# ===== Integration executable =====
add_executable(simulation
    apps/simulation.cpp
)

target_link_libraries(simulation PRIVATE core)

# ===== Unit tests =====
include(CTest)
enable_testing()

find_package(Catch2 CONFIG REQUIRED)

add_executable(unit_tests
    tests/test_ringbuffer.cpp
    tests/test_realtimesim.cpp
    tests/test_audioengine.cpp
)

target_link_libraries(unit_tests PRIVATE core Catch2::Catch2WithMain)

include(Catch)

catch_discover_tests(unit_tests)
