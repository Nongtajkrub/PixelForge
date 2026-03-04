#include "parser.hpp"
#include "diagnostic.hpp"
#include "token.hpp"
#include "ast.hpp"

#include <optional>
#include <iostream>
#include <tuple>
#include <vector>

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

	ENSURE_NOT_EOF();

	// Ensure an identifier follow var.
	if (!expect(TokenKind::IDENTIFIER, DiagnosticKind::UNEXPECTED_TOKEN)) {
		return std::nullopt;
	}
	node->name = new_primary_node(ASTNodeKind::IDENTIFIER, this->tokens.prev());

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
	if (!expect(TokenKind::IDENTIFIER, DiagnosticKind::UNEXPECTED_TOKEN)) {
		return std::nullopt;
	}
	node->name = new_primary_node(ASTNodeKind::IDENTIFIER, this->tokens.prev());

	ENSURE_NOT_EOF();

	if (!expect(TokenKind::LEFT_BRACE, DiagnosticKind::UNEXPECTED_TOKEN)) {
		return std::nullopt;
	}

	// Parse function arguments.
	if (!parse_func_args(node->args)) {
		return std::nullopt;
	}

	if (!expect(TokenKind::SEMICOLON, DiagnosticKind::EXPECTED_SEMICOLON)) {
		return std::nullopt;
	}

	// Parse function body.
	if (!parse_block(TokenKind::ENDFUNC, node->body)) {
		return std::nullopt;
	}

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
	case TokenKind::LEFT_BRACE:
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
	case TokenKind::LEFT_BRACE: {
		if (*left.adr != ASTNodeKind::IDENTIFIER) {
			return std::nullopt;
		}

		auto node = new_node<CallExpr>(ASTNodeKind::CALL);
		node->name = left;
		if (!parse_func_args(node->args)) {
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
			Diagnostic(DiagnosticKind::UNEXPECTED_END_OF_FILE, this->tokens.prev())
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

bool Parser::parse_func_args(std::vector<ASTNode>& buf) {
	while (true) {
		if (this->tokens.is_eof()) {
			Diagnostic(DiagnosticKind::UNEXPECTED_END_OF_FILE, this->tokens.prev())
				.emit(std::cout);
			return false;
		}
		// Check prev because parse_expr consume the terminator token.
		if (this->tokens.prev().kind == TokenKind::RIGHT_BRACE) {
			return true;
		}

		auto token = this->tokens.peek();

		if (token.kind == TokenKind::COMMA) {
			continue;
		}

		if (auto node = parse_expr(); !node) {
			return false;
		} else {
			buf.push_back(*node);
		}
	}
}

;} // namespace scr
