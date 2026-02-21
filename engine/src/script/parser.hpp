#pragma once

#include "../util/bump_arena.hpp"
#include "source_stream.hpp"
#include "token.hpp"
#include "ast.hpp"
#include "diagnostic.hpp"

#include <cstddef>
#include <optional>
#include <vector>
#include <iostream>

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

	inline const ASTNode get_ast(size_t line) {
		return this->ast[line - 1];
	}

private:
	std::optional<ASTNode> parse_stmt();
	std::optional<ASTNode> parse_declaration_stmt();
	std::optional<ASTNode> parse_expr();
	std::optional<ASTNode> parse_math_expr();
	std::optional<ASTNode> pratt_parser(u8 min_bp);

	inline bool expect(TokenKind kind, DiagnosticKind err) {
		if (match_token(kind)) {
			return true;
		} else {
			Diagnostic(err, this->tokens.peek())
				.emit(std::cout);
			return false;
		}
	}

	inline bool expect_peek(TokenKind kind, DiagnosticKind err) {
		if (this->tokens.peek().kind == kind) {
			return true;
		} else {
			Diagnostic(err, this->tokens.peek())
				.emit(std::cout);
			return false;
		}
	}

	inline bool match_token(TokenKind kind) {
		return this->tokens.match(
			[kind](auto token) -> bool { return token.kind == kind; });
	}

	inline ASTNode new_primary_node(ASTNodeKind kind, const Token& token) {
		auto node = this->arena.alloc<PrimaryExpr>();
		node->token = token;
		return ASTNode(&node->kind);
	}

	template <typename  T>
	inline T* new_node(ASTNodeKind kind) {
		auto node = this->arena.alloc<T>();
		node->kind = kind;
		return node;
	}
};

} // namespace scr
