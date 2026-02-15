#pragma once

#include "../util/bump_arena.hpp"
#include "source_stream.hpp"
#include "token.hpp"
#include "ast.hpp"

#include <cstddef>
#include <optional>
#include <vector>

namespace scr {

class Parser {
private:
	// Arena allocator for nodes.
	BumpArena& arena;

	SourceStream<const std::span<Token>, Token, const Token&> tokens;

	// Abstract Syntax Tree separated into each line.
	std::vector<ASTNode> ast;

public:
	Parser(const std::span<Token> tokens, BumpArena& arena) :
		tokens(tokens),
		arena(arena)
	{ }

	bool parse();

	inline const std::span<ASTNode> get_ast() {
		return this->ast;
	} 

private:
	std::optional<ASTNode> parse_stmt();
	std::optional<ASTNode> parse_declaration_stmt();
	std::optional<ASTNode> parse_expr();

	inline bool match_token(TokenKind kind) {
		return this->tokens.match(
			[kind](auto token) -> bool { return token.kind == kind; });
	}
};

} // namespace scr
