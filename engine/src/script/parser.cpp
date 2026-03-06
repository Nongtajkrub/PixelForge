#include "parser.hpp"
#include "diagnostic.hpp"
#include "token.hpp"
#include "ast.hpp"

#include <cassert>
#include <optional>
#include <iostream>
#include <tuple>
#include <utility>
#include <vector>

#define ENSURE_NOT_EOF()                                                      \
do                                                                            \
if (this->tokens.is_eof()) {                                                  \
	Diagnostic(DiagnosticKind::UNEXPECTED_EOF, this->tokens.prev())           \
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
	switch (this->tokens.peek().kind) {
	case TokenKind::PASS:
		return parse_nop();
	case TokenKind::LET:
		return parse_var_declaration_stmt();
	case TokenKind::FUNC:
		return parse_func_declaration_stmt();
	case TokenKind::IDENTIFIER:
		return parse_expr();
	default:
		Diagnostic(DiagnosticKind::UNEXPECTED_TOKEN, this->tokens.advance())
			.emit(std::cout);
		return std::nullopt;
	}
}

std::optional<ASTNode> Parser::parse_nop() {
	auto node = new_node<NopNode>(ASTNodeKind::NOP);
	this->tokens.advance();
	if (!expect(TokenKind::SEMICOLON, DiagnosticKind::EXPECTED_SEMICOLON)) {
		return std::nullopt;
	}
	return ASTNode(&node->kind);
}

std::optional<ASTNode> Parser::parse_var_declaration_stmt() {
	auto node = new_node<VarDeclarationStmt>(ASTNodeKind::VAR_DECLARATION);

	// Ignore "var" keyword.
	this->tokens.advance();

	if (!parse_type_annotation<VarDeclarationStmt*>(node)) {
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

std::optional<ASTNode> Parser::parse_func_declaration_stmt() {
	auto node = new_node<FuncDeclarationStmt>(ASTNodeKind::FUNC_DECLARATION);

	// Ignore "func" keyword.
	this->tokens.advance();

	ENSURE_NOT_EOF();

	// Ensure an identifier follow func.
	if (!expect_peek(
			TokenKind::IDENTIFIER, DiagnosticKind::EXPECTED_IDENTIFIER)) {
		std::println("Found: {}", this->tokens.advance().kind_as_str());
		return std::nullopt;
	}
	node->name =
		new_primary_node(ASTNodeKind::IDENTIFIER, this->tokens.advance());

	ENSURE_NOT_EOF();

	if (!expect(TokenKind::LEFT_PAREN, DiagnosticKind::EXPECTED_LEFT_PAREN)) {
		return std::nullopt;
	}

	// Parse function arguments.
	if (!parse_func_args(node->args)) {
		return std::nullopt;
	}

	ENSURE_NOT_EOF();

	if (!expect(TokenKind::SEMICOLON, DiagnosticKind::EXPECTED_SEMICOLON)) {
		return std::nullopt;
	}

	// Parse function body.
	if (!parse_block(TokenKind::ENDFUNC, node->body)) {
		return std::nullopt;
	}

	ENSURE_NOT_EOF();

	if (!expect(TokenKind::SEMICOLON, DiagnosticKind::EXPECTED_SEMICOLON)) {
		return std::nullopt;
	}

	return ASTNode(&node->kind);
}

// Will consume the expression terminator token.
std::optional<ASTNode> Parser::parse_expr() {
	auto node = pratt_parser();
	if (!node) {
		return std::nullopt;
	}

	// Concumse expression terminator token. (';', ')', ...)
	this->tokens.advance();

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
	case TokenKind::LEFT_PAREN:
		return {5, 6};
	default:
		return {0, 0};
	}
}

std::optional<ASTNode> Parser::pratt_nud() {
	switch (this->tokens.peek().kind) {
	case TokenKind::NUMBER:
	case TokenKind::STRING:
		return new_primary_node(ASTNodeKind::LITERAL, this->tokens.advance());
	case TokenKind::IDENTIFIER:
		return new_primary_node(ASTNodeKind::IDENTIFIER, this->tokens.advance());
	default:
		Diagnostic(DiagnosticKind::UNEXPECTED_TOKEN, this->tokens.advance())
			.emit(std::cout);
		return std::nullopt;
	}
}

std::optional<ASTNode> Parser::pratt_led(Token op, ASTNode left, u8 min_bp) {
	switch (op.kind) {
	case TokenKind::PLUS:
	case TokenKind::MINUS:
	case TokenKind::STAR:
	case TokenKind::SLASH: {
		auto node = new_node<BinaryExpr>(ASTNodeKind::BINARY);
		node->left = left;
		node->op = op;
		OPT_ASSIGN_OR_RETURN(node->right, pratt_parser(min_bp));
		return ASTNode(&node->kind);
	}
	case TokenKind::LEFT_PAREN: {
		if (*left.adr != ASTNodeKind::IDENTIFIER) {
			Diagnostic(DiagnosticKind::EXPECTED_IDENTIFIER, op).emit(std::cout);
			return std::nullopt;
		}

		auto node = new_node<CallExpr>(ASTNodeKind::CALL);
		node->name = left;
		if (!parse_func_args(node->args, true)) {
			return std::nullopt;
		}

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

	while (true) {
		ENSURE_NOT_EOF();

		const Token op = this->tokens.peek();

		const auto [lbp, rbp] = resolve_binding_power(op.kind);
		if (lbp <= min_bp) {
			break;
		}

		this->tokens.advance();
		OPT_ASSIGN_OR_RETURN(left, pratt_led(op, *left, rbp));
	}

	return left;
}

bool Parser::parse_block(TokenKind end, std::vector<ASTNode>& buf) {
	while (true) {
		if (this->tokens.is_eof()) {
			Diagnostic(DiagnosticKind::UNEXPECTED_EOF, this->tokens.prev())
				.emit(std::cout);
			return false;
		}

		if (this->tokens.peek().kind == end) {
			this->tokens.advance();
			return true;
		}

		if (auto stmt = parse_stmt(); !stmt) {
			return false;
		} else {
			buf.push_back(*stmt);
		}
	}
}

bool Parser::parse_func_args(std::vector<ASTNode>& buf, bool as_expr) {
	while (true) {
		ENSURE_NOT_EOF_BOOL();

		if (match_token(TokenKind::RIGHT_PAREN)) {
			return true;
		}

		if (!as_expr) {
			auto node = new_node<FuncArgument>(ASTNodeKind::FUNC_ARGUMENTS);

			if (!parse_type_annotation<FuncArgument*>(node)) {
				return false;
			}
			buf.push_back(std::move(ASTNode(&node->kind)));
		} else {
			if (auto expr = pratt_parser(); !expr) { 
				return false;
			} else {
				buf.push_back(std::move(*expr));
			}
		}

		ENSURE_NOT_EOF_BOOL();

		// RIGHT_PAREN will be handle in the next iteration of the loop.
		if (auto kind = this->tokens.peek().kind; kind == TokenKind::COMMA) { 
			this->tokens.advance();
		} else if (kind != TokenKind::RIGHT_PAREN) {
			Diagnostic(DiagnosticKind::EXPECTED_COMMA, this->tokens.peek())
				.emit(std::cout);
			return false;
		}
	}
}

;} // namespace scr
