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

	src/core/io/file.hpp
	src/core/io/file.cpp
	src/core/io/byte.hpp
	src/core/io/byte.cpp
	src/core/arena/bump_arena.hpp

	src/script/common/token.hpp
	src/script/common/token.cpp
	src/script/common/diagnostic.hpp
	src/script/common/diagnostic.cpp
	src/script/common/source_stream.hpp
	src/script/common/symbol_table.cpp
	src/script/common/symbol_table.hpp
	src/script/lexer/lexer.hpp
	src/script/lexer/lexer.cpp
	src/script/parser/parser.hpp
	src/script/parser/parser.cpp
	src/script/ast/ast.hpp
	src/script/ast/ast.cpp
)
