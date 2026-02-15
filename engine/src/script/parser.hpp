#pragma once

#include "../util/bump_arena.hpp"
#include "source_stream.hpp"
#include "token.hpp"
#include "ast.hpp"
#include "diagnostic.hpp"

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

	inline bool expect_semicolon() {
		if (match_token(TokenKind::SEMICOLON)) {
			return true;
		} else {
			Diagnostic(DiagnosticKind::EXPECTED_SEMICOLON, this->tokens.peek())
				.emit(std::cout);
			return false;
		}
	}

	template <typename T>
	inline ASTNode new_node(ASTNodeKind kind, const Token& token) {
		auto node = this->arena.alloc<T>();
		node->kind = kind;
		node->token = token;
		return ASTNode(&node->kind);
	}

	inline bool match_token(TokenKind kind) {
		return this->tokens.match(
			[kind](auto token) -> bool { return token.kind == kind; });
	}
};

} // namespace scr
