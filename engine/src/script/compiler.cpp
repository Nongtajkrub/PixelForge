#include "compiler.hpp"

#include "../core/cplusplus/container/bump_arena.hpp"
#include "../core/cplusplus/io/file_io.hpp"
#include "../core/cplusplus/io/byte_io.hpp"
#include "code_generator.hpp"
#include "preprocessor.hpp"
#include "const_pool.hpp"
#include "diagnostic.hpp"
#include "parser.hpp"
#include "packer.hpp"
#include "lexer.hpp"
#include "vm/deserializer.h"

#include <cstddef>
#include <fstream>

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

	const auto serialize_cpool = const_poool.serialize();
	bytes_output(serialize_cpool, std::cout);

	/*
	for (auto ast : parser.get_ast()) {
		ast.output(std::cout);
	}
	*/
	
	auto code_generator = CodeGenerator(parser.get_ast());

	code_generator.generate();
	//code_generator.output_code(std::cout);
	
	const auto cpool_bytes = const_poool.serialize();
	const auto code_bytes = code_generator.serialize();

	const auto packed = pack(cpool_bytes, code_bytes);
	deserialize((char*)packed.data(), packed.size());

	auto file = std::ofstream("script.out");
	//code_generator.output_serialize(file, code_generator.serialize());

	return true;
}

}; // namespace scr
