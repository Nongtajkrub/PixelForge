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
		if (auto node = parse_stmt(); !node) {
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
	ENSURE_NOT_EOF();

	switch (this->tokens.advance().kind) {
	case TokenKind::IDENTIFIER: 
		return new_primary_node(ASTNodeKind::IDENTIFIER, this->tokens.prev());
	case TokenKind::NUMBER:
	case TokenKind::STRING:
		return new_primary_node(ASTNodeKind::LITERAL, this->tokens.prev());
	case TokenKind::MATH:
		return parse_math_expr();
	default:
		Diagnostic(DiagnosticKind::EXPECTED_EXPRESSION, this->tokens.prev())
			.emit(std::cout);
		return std::nullopt;
	}
}

std::optional<ASTNode> Parser::parse_math_expr() {
	if (!expect(
			TokenKind::LEFT_BRACKET, DiagnosticKind::EXPECTED_LEFT_BRACKET)) {
		return std::nullopt;
	}

	auto ast = pratt_parser(0);

	if (!expect(
			TokenKind::RIGHT_BRACKET, DiagnosticKind::EXPECTED_RIGHT_BRACKET)) {
		return std::nullopt;
	}
	if (!expect(TokenKind::SEMICOLON, DiagnosticKind::EXPECTED_SEMICOLON)) {
		return std::nullopt;
	}

	return ast;
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

std::optional<ASTNode> Parser::pratt_parser(u8 min_bp) {
	auto node = new_node<BinaryExpr>(ASTNodeKind::BINARY);

	ENSURE_NOT_EOF();

	// Parse the left-hand side as a primary (atomic) expression.
	switch (this->tokens.advance().kind) {
	case TokenKind::NUMBER:
		node->left = new_primary_node(ASTNodeKind::LITERAL, this->tokens.prev());
		break;
	default:
		Diagnostic(DiagnosticKind::EXPECTED_ARITHMETIC, this->tokens.prev())
			.emit(std::cout);
		return std::nullopt;
	}

	ENSURE_NOT_EOF();

	// Validate that the current token is a valid arithmetic operator.
	if (!this->tokens.advance().is_arithmetic_operator()) {
		Diagnostic(DiagnosticKind::UNEXPECTED_TOKEN, this->tokens.prev())
			.emit(std::cout);
		return std::nullopt;
	}
	node->op = this->tokens.prev();

	if (this->tokens.peek().kind == TokenKind::LEFT_BRACE) {
		OPT_ASSIGN_OR_RETURN(node->right, pratt_parser(0), rhs);
	}

	// Validate that a number follow an arithmetic operator.
	if (!expect_peek(TokenKind::NUMBER, DiagnosticKind::EXPECTED_NUMBER)) {
		return std::nullopt;
	}

	// If the next expression is a literal parse it directly as the right-hand side.
	if (this->tokens.can_look_ahead(1) 
			&& !this->tokens.look_ahead(1).is_arithmetic_operator()) {
		node->right =
			new_primary_node(ASTNodeKind::LITERAL, this->tokens.advance());
		return ASTNode(&node->kind);
	}

	// Recursively parse the right-hand side based on operator binding power.
	auto [lbp, rbp] = resolve_binding_power(this->tokens.prev().kind); 
	if (lbp > min_bp) {
		OPT_ASSIGN_OR_RETURN(node->right, pratt_parser(rbp), rhs);
	}

	return ASTNode(&node->kind);
}

} // namespace scr
