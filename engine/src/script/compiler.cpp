#include "compiler.hpp"

#include "../core/cplusplus/io/file_io.hpp"
#include "../core/cplusplus/container/bump_arena.hpp"
#include "code_generator.hpp"
#include "const_pool.hpp"
#include "diagnostic.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "preprocessor.hpp"

#include <cstddef>

namespace scr {

using namespace core;

static constexpr size_t DEFAULT_NODES_ARENA_SIZE = 2048;

bool Compiler::compile() {
	const auto source = fload_str(this->src_path);
	if (!source) {
		emit(DiagnosticKind::FAIL_OPEN_SOURCE);
		return false;
	}

	auto lexer = Lexer(*source, this->err_stream);
	if (!lexer.lex()) {
		return false;
	}

	if (!(Preprocessor(
			lexer.get_token(), this->symbols, this->err_stream).process())) {
		return false;
	}

	auto nodes_arena = BumpArena(DEFAULT_NODES_ARENA_SIZE); 
	auto const_poool = ConstPool();

	auto parser =
		Parser(
			lexer.get_token(),
			this->symbols, const_poool, nodes_arena, this->err_stream);
	if (!parser.parse()) {
		return false;
	}

	/*
	for (auto ast : parser.get_ast()) {
		ast.output(std::cout);
	}
	*/
	
	auto code_generator = CodeGenerator(parser.get_ast());

	code_generator.generate();
	code_generator.output_code(std::cout);

	return true;
}

}; // namespace scr
