#include "parser.hpp"
#include "diagnostic.hpp"
#include "token.hpp"
#include "ast.hpp"

#include <optional>
#include <iostream>
#include <tuple>

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
		// Recursively parse the code.
		if (auto node = parse_stmt(); node) {
			this->ast.push_back(*node);
		} else {
			return false;
		}
	}
	return true;
}

std::optional<ASTNode> Parser::parse_stmt() {
	if (match_token(TokenKind::VAR)) {
		return parse_declaration_stmt();
	}
	Diagnostic(DiagnosticKind::UNEXPECTED_TOKEN, this->tokens.advance())
		.emit(std::cout);
	return std::nullopt;
}

std::optional<ASTNode> Parser::parse_declaration_stmt() {
	auto node = new_node<DeclarationStmt>(ASTNodeKind::DECLARATION);

	ENSURE_NOT_EOF();

	// Ensure an identifier follow var.
	if (match_token(TokenKind::IDENTIFIER)) {
		node->name = this->tokens.prev();
	} else {
		Diagnostic(DiagnosticKind::UNEXPECTED_TOKEN, this->tokens.prev())
			.emit(std::cout);
		return std::nullopt;
	}

	ENSURE_NOT_EOF();

	// Resolve whether the declaration statment initialize anything.
	switch (this->tokens.advance().kind) {
	case TokenKind::SEMICOLON:
		node->init = std::nullopt;
		return ASTNode(&node->kind);
	case TokenKind::EQUAL:
		if (node->init = parse_expr(); !node->init) {
			return std::nullopt;
		}
		return ASTNode(&node->kind);
	default:
		Diagnostic(DiagnosticKind::UNEXPECTED_TOKEN, this->tokens.prev())
			.emit(std::cout);
		return std::nullopt;
	}

}

std::optional<ASTNode> Parser::parse_expr() {
	auto node = pratt_parser();
	if (!node) {
		return std::nullopt;
	}

	ENSURE_NOT_EOF();

	if (!expect(TokenKind::SEMICOLON, DiagnosticKind::EXPECTED_SEMICOLON) 
			|| !node) {
		return std::nullopt;
	}

	return node;
}

static std::tuple<u8, u8> resolve_binding_power(TokenKind kind) {
	switch (kind) {
	case TokenKind::PLUS:
	case TokenKind::MINUS:
		return {1, 2};
	case TokenKind::STAR:
	case TokenKind::SLASH:
		return {3, 4};
	default:
		return {0, 0};
	}
}

std::optional<ASTNode> Parser::pratt_nud() {
	switch (this->tokens.peek().kind) {
	case TokenKind::NUMBER:
	case TokenKind::STRING:
		return new_primary_node(ASTNodeKind::LITERAL, this->tokens.advance());
	default:
		Diagnostic(DiagnosticKind::UNEXPECTED_TOKEN, this->tokens.advance())
			.emit(std::cout);
		return std::nullopt;
	}
}

std::optional<ASTNode> Parser::pratt_led(Token op, ASTNode left, u8 min_bp) {
	switch (this->tokens.peek().kind) {
	case TokenKind::NUMBER: {
		auto node = new_node<BinaryExpr>(ASTNodeKind::BINARY);
		node->left = left;
		node->op = op;
		OPT_ASSIGN_OR_RETURN(node->right, pratt_parser(min_bp));
		return ASTNode(&node->kind);
	}
	default:
		Diagnostic(DiagnosticKind::UNEXPECTED_TOKEN, this->tokens.advance())
			.emit(std::cout);
		return std::nullopt;
	}
}

std::optional<ASTNode> Parser::pratt_parser(u8 min_bp) {
	auto left = pratt_nud();
	if (!left) {
		return std::nullopt;
	}

	while (!this->tokens.is_eof()) {
		const Token op = this->tokens.peek();
		if (op.kind == TokenKind::SEMICOLON) {
			break;
		}

		const auto [lbp, rbp] = resolve_binding_power(op.kind);
		if (lbp < min_bp) {
			break;
		}

		this->tokens.advance();
		OPT_ASSIGN_OR_RETURN(left, pratt_led(op, *left, rbp));
	}
	
	return left;
}

} // namespace scr
