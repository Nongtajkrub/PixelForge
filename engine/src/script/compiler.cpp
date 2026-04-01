#include "compiler.hpp"
#include "diagnostic.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "preprocessor.hpp"
#include "../core/file_io.hpp"
#include "../core/bump_arena.hpp"

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

	auto parser =
		Parser(lexer.get_token(), this->symbols, nodes_arena, this->err_stream);
	if (!parser.parse()) {
		return false;
	}

	for (auto ast : parser.get_ast()) {
		scr::ast_output(std::cout, ast);
	}

	return true;
}

}; // namespace scr
