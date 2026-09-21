#include <engine/forgescript/fscript_compiler.hpp>

#include <core-cplusplus/utilities/bump_arena.hpp>
#include <core-cplusplus/utilities/pipeline.hpp>
#include <core-cplusplus/io/file_io.hpp>

#include "fscript_code_generator.hpp"
#include "fscript_preprocessor.hpp"
#include "fscript_symbol_table.hpp"
#include "fscript_const_pool.hpp"
#include "fscript_diagnostic.hpp"
#include "fscript_parser.hpp"
#include "fscript_packer.hpp"
#include "fscript_lexer.hpp"

#include <filesystem>
#include <optional>
#include <cstddef>
#include <ostream>
#include <print>
#include <vector>

namespace scr {

struct PipelineCtx {
	std::ostream& err_stream;

	core::BumpArena arena;
	SymbolTable symbols;
	ConstPool cpool;
};

struct load_soruce {
	core::PipelineOut<std::string> operator()(
		const std::filesystem::path& src, PipelineCtx& ctx) const {
		const auto data = core::fload_str(src);

		if (!data) {
			Diagnostic(DiagnosticKind::FAIL_OPEN_SOURCE).emit(ctx.err_stream);
			return core::plterminate;
		}

		return data;
	}
};

struct lex_source {
	core::PipelineOut<TokenBuffer> operator()(
		std::string src, PipelineCtx& ctx) const {
		const auto tokens = lex(src, ctx.err_stream);

		if (!tokens) {
			return core::plterminate;
		}

		return tokens;
	}
};

struct preprocess_tokens {
	core::PipelineOut<TokenBuffer> operator()(
		TokenBuffer tokens, PipelineCtx& ctx) const {
		auto preprocessor = Preprocessor(tokens, ctx.symbols, ctx.err_stream);

		if (!preprocessor.process()) {
			return core::plterminate;
		}

		return tokens;
	}
};

struct parse_tokens {
	core::PipelineOut<ASTBuffer> operator()(
		TokenBuffer tokens, PipelineCtx& ctx) const {
		ASTBuffer ast{};
		auto parser = Parser(
			tokens, ctx.symbols, ctx.cpool, ctx.arena, ast, ctx.err_stream); 

		if (!parser.parse()) {
			return core::plterminate;
		}

		return ast;
	}
}; 

struct generate_code {
	core::PipelineOut<std::vector<u8>> operator()(
		ASTBuffer ast, PipelineCtx& ctx) const {
		auto generator = CodeGenerator(ast);

		generator.generate();

		return generator.serialize();
	}
};

struct package_code {
	inline core::PipelineOut<CodePackage> operator()(
		std::vector<u8> code, PipelineCtx& ctx) const {
		const auto package = pack(ctx.cpool.serialize(), code);

		for (const auto byte : package) {
			std::println("{}", byte);
		}

		return package;
	}
};

std::optional<std::vector<u8>> Compiler::compile() {
	auto ctx = (PipelineCtx) {
		.err_stream = this->err_stream,
		.arena = core::BumpArena(2048),
		.symbols = SymbolTable(),
		.cpool = ConstPool(),
	};

	core::Pipeline<
		load_soruce,
		lex_source,
		preprocess_tokens,	
		parse_tokens, generate_code, package_code>::execute(this->src, ctx);

	return std::nullopt;
}

}; // namespace scr
