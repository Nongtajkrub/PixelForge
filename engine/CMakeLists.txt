add_library(Engine STATIC)

target_sources(Engine
	PRIVATE
		src/forgescript/vm/fscript_instruction.c
	
		src/forgescript/fscript_ast.cpp	
		src/forgescript/fscript_ast.cpp
		src/forgescript/fscript_code_generator.cpp
		src/forgescript/fscript_compiler.cpp
		src/forgescript/fscript_const_pool.cpp
		src/forgescript/fscript_diagnostic.cpp
		src/forgescript/fscript_lexer.cpp
		src/forgescript/fscript_packer.cpp
		src/forgescript/fscript_parser.cpp
		src/forgescript/fscript_preprocessor.cpp
		src/forgescript/fscript_symbol_table.cpp
		src/forgescript/fscript_token.cpp
)

target_include_directories(Engine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(Engine
	PUBLIC
		Core
)
