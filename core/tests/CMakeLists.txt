cmake_minimum_required(VERSION 3.15...3.28)

project(PixelForgeCoreTests LANGUAGES CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_subdirectory(.. core_lib_build)

add_executable(CBumpArenaTests)

target_sources(CBumpArenaTests
	PRIVATE
		bump_arena.test.c
)

target_link_libraries(CBumpArenaTests
	PRIVATE
		Core
)

set_target_properties(CBumpArenaTests PROPERTIES OUTPUT_NAME "CBumpArenaTests")
