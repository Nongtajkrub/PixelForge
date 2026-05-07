#include "fscript_compiler.hpp"

#include "../core/cplusplus/container/bump_arena.hpp"
#include "../core/cplusplus/io/file_io.hpp"
#include "fscript_code_generator.hpp"
#include "fscript_preprocessor.hpp"
#include "fscript_const_pool.hpp"
#include "fscript_diagnostic.hpp"
#include "fscript_parser.hpp"
#include "fscript_packer.hpp"
#include "fscript_lexer.hpp"

#include <cstddef>
#include <optional>
#include <iostream>

namespace scr {

using namespace core;

static constexpr size_t DEFAULT_NODES_ARENA_SIZE = 2048;

std::optional<std::vector<u8>> Compiler::compile() {
	const auto source = fload_str(this->src_path);
	if (!source) {
		emit(DiagnosticKind::FAIL_OPEN_SOURCE);
		return std::nullopt;
	}

	auto lexer = Lexer(*source, this->err_stream);
	if (!lexer.lex()) {
		return std::nullopt;
	}

	if (!(Preprocessor(
			lexer.get_token(), this->symbols, this->err_stream).process())) {
		return std::nullopt;
	}

	auto arena = BumpArena(DEFAULT_NODES_ARENA_SIZE); 
	auto cpool = ConstPool();

	auto parser =
		Parser(
			lexer.get_token(), this->symbols, cpool, arena, this->err_stream);
	if (!parser.parse()) {
		return std::nullopt;
	}

	/*
	for (const auto& node : parser.get_ast()) {
		node.output(std::cout);
	}
	*/

	auto code_gen = CodeGenerator(parser.get_ast());

	code_gen.output_code(std::cout);

	return pack(cpool, code_gen);
}

}; // namespace scr
