#include "parser.hpp"
#include "diagnostic.hpp"
#include "token.hpp"
#include "ast.hpp"

#include <optional>
#include <iostream>

#define ENSURE_NOT_EOF()                                                      \
do                                                                            \
if (this->tokens.is_eof()) {                                                  \
	Diagnostic(DiagnosticKind::UNEXPECTED_END_OF_FILE, this->tokens.prev())   \
		.emit(std::cout);                                                     \
	return std::nullopt;                                                      \
}                                                                             \
while (0)                                                                     

namespace scr {

bool Parser::parse() {
	while (!this->tokens.is_eof()) {
		auto node = parse_stmt();

		if (!node.has_value()) {
			return false;
		}

		this->ast.push_back(node.value());
	}

	return true;
}

std::optional<ASTNode> Parser::parse_stmt() {
	if (match_token(TokenKind::VAR)) {
		return parse_declaration_stmt();
	}
	return std::nullopt;
}

std::optional<ASTNode> Parser::parse_declaration_stmt() {
	auto node = this->arena.alloc<DeclarationStmt>();
	node->kind = ASTNodeKind::DECLARATION;

	ENSURE_NOT_EOF();

	if (match_token(TokenKind::IDENTIFIER)) {
		node->name = this->tokens.prev();
	} else {
		Diagnostic(DiagnosticKind::UNEXPECTED_TOKEN, this->tokens.prev())
			.emit(std::cout);
		return std::nullopt;
	}

	ENSURE_NOT_EOF();

	switch (this->tokens.advance().kind) {
	case TokenKind::SEMICOLON:
		node->init = std::nullopt;
		break;
	case TokenKind::EQUAL:
		node->init = parse_expr();
		break;
	default:
		Diagnostic(DiagnosticKind::UNEXPECTED_TOKEN, this->tokens.prev())
			.emit(std::cout);
		return std::nullopt;
	}

	return ASTNode(&node->kind);
}

std::optional<ASTNode> Parser::parse_expr() {
	// TODO: Implement
	return std::nullopt;
}

} // namespace scr
