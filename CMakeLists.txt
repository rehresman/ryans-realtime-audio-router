cmake_minimum_required(VERSION 3.20)
project(rt_audio_router LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(ryans-realtime-audio-router
    src/main.cpp
    src/AudioEngine.cpp
)

target_include_directories(ryans-realtime-audio-router PRIVATE include)

# Helpful warnings
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
  target_compile_options(ryans-realtime-audio-router PRIVATE -Wall -Wextra -Wpedantic)
endif()
