cmake_minimum_required(VERSION 3.15)

project(gameboy-engine)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_executable(engine)

target_sources(engine
	PRIVATE

	src/main.cpp
	src/global.hpp

	src/core/file_io.hpp
	src/core/file_io.cpp
	src/core/byte_io.hpp
	src/core/byte_io.cpp
	src/core/bump_arena.hpp

	src/script/token.hpp
	src/script/token.cpp
	src/script/diagnostic.hpp
	src/script/diagnostic.cpp
	src/script/source_stream.hpp
	src/script/symbol_table.cpp
	src/script/symbol_table.hpp
	src/script/command_props.cpp
	src/script/command_props.hpp
	src/script/lexer.hpp
	src/script/lexer.cpp
	src/script/preprocessor.hpp
	src/script/preprocessor.cpp
	src/script/parser.hpp
	src/script/parser.cpp
	src/script/ast.hpp
	src/script/ast.cpp
)
