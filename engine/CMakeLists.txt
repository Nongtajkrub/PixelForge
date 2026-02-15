cmake_minimum_required(VERSION 3.15)

project(gameboy-engine)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_executable(engine)

target_sources(engine
	PRIVATE

	src/main.cpp

	src/io/file.hpp
	src/io/file.cpp
	src/io/byte.hpp
	src/io/byte.cpp

	src/instruction/type.hpp
	src/instruction/block.hpp
	src/instruction/group.cpp
	src/instruction/group.hpp

	src/script/token.hpp
	src/script/token.cpp
	src/script/lexer.hpp
	src/script/lexer.cpp
	src/script/source_stream.hpp
	src/script/parser.hpp
	src/script/parser.cpp
	src/script/ast.hpp
	src/script/ast.cpp
	src/script/diagnostic.hpp
	src/script/diagnostic.cpp

	src/global.hpp

	src/instruction/block.cpp
)
