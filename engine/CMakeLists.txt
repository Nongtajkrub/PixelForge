cmake_minimum_required(VERSION 3.15)

project(gameboy_engine)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# core
add_library(core)
file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS src/core/*)
target_sources(core PRIVATE ${CORE_SOURCES})
target_include_directories(core PUBLIC src)
target_compile_features(core PUBLIC cxx_std_23)

# script
add_library(script)
file(GLOB_RECURSE SCRIPT_SOURCES CONFIGURE_DEPENDS src/script/*)
target_sources(script PRIVATE ${SCRIPT_SOURCES})
target_include_directories(script PUBLIC src)
target_compile_features(script PUBLIC cxx_std_23)

# engine
add_executable(engine src/main.cpp)
target_link_libraries(engine PRIVATE core script)
target_compile_features(engine PRIVATE cxx_std_23)

# enable warning
if (MSVC)
    target_compile_options(engine PRIVATE /W4)
else()
    target_compile_options(engine PRIVATE -Wall -Wextra -Wpedantic)
endif()
