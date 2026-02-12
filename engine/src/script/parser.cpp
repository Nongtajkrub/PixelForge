#include "parser.hpp"

#include "token.hpp"
#include "ast.hpp"
#include <optional>

namespace scr {

void Parser::parse() {
	Token& token = this->tokens.advance();
	while (!this->tokens.is_eof()) {
		switch (token.kind) {
		case TokenKind::VAR:
			parse_declaration_stmt();
			break;
		default:
			break;
		}

		token = this->tokens.advance();
	}
}

std::optional<ASTNode> Parser::parse_declaration_stmt() {
	auto node = this->arena.alloc<DeclarationStmt>();

	if (Token& token = this->tokens.advance(); token.kind != TokenKind::IDENTIFIER) {
		// TODO: Implement error returning.
		return std::nullopt;
	} else {
		node->name = token;
	}

	switch (this->tokens.advance().kind) {
		case TokenKind::SEMICOLON:
			node->init = nullptr;
			return ASTNode(&node->kind);
		case TokenKind::EQUAL:
			parse_expr();
			break;
		default:
			// TODO: Implement error returning.
	}

	return ASTNode(&node->kind);
}

std::optional<ASTNode> Parser::parse_expr() {
	// TODO: Implement
	return std::nullopt;
}

} // namespace scr
