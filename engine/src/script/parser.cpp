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
	case TokenKind::IF:
		return parse_if_stmt();
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
	OPT_ASSIGN_OR_RETURN(
		node->body,
		parse_block(
			[](auto kind) -> bool { return kind == TokenKind::ENDFUNC; }));

	// Ignore the "endfunc" keyword.
	this->tokens.advance();

	ENSURE_NOT_EOF();

	if (!expect(TokenKind::SEMICOLON, DiagnosticKind::EXPECTED_SEMICOLON)) {
		return std::nullopt;
	}

	return ASTNode(&node->kind);
}

std::optional<ASTNode> Parser::parse_if_stmt() {
	auto node = new_node<IfStmt>(ASTNodeKind::IF);

	// Ignore "if" keyword.
	this->tokens.advance();

	ENSURE_NOT_EOF();

	if (!expect(TokenKind::LEFT_PAREN, DiagnosticKind::EXPECTED_LEFT_PAREN)) {
		return std::nullopt;
	}

	OPT_ASSIGN_OR_RETURN(node->expr, pratt_parser());

	ENSURE_NOT_EOF();

	if (!expect(TokenKind::RIGHT_PAREN, DiagnosticKind::EXPECTED_RIGHT_PAREN)) {
		return std::nullopt;
	}

	ENSURE_NOT_EOF();

	if (!expect(TokenKind::SEMICOLON, DiagnosticKind::EXPECTED_SEMICOLON)) {
		return std::nullopt;
	}

	// parse then branch.
	OPT_ASSIGN_OR_RETURN(
		node->then_branch,
		parse_block(
			[](auto kind) -> bool {
				return kind == TokenKind::ENDIF || kind == TokenKind::ELSE; }));

	ENSURE_NOT_EOF();

	// parse else branch if exist.
	if (match_token(TokenKind::ELSE)) {
		ENSURE_NOT_EOF();

		if (!expect(TokenKind::SEMICOLON, DiagnosticKind::EXPECTED_SEMICOLON)) {
			return std::nullopt;
		}

		OPT_ASSIGN_OR_RETURN(
			node->else_branch,
			parse_block(
				[](auto kind) -> bool { return kind == TokenKind::ENDIF; }));
	} else {
		node->else_branch = std::nullopt;
	}

	// Ignore "endif" keyword.
	this->tokens.advance();

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
	case TokenKind::DOUBLE_EQUAL:
		return {1, 2};
	case TokenKind::PLUS:
	case TokenKind::MINUS:
		return {2, 3};
	case TokenKind::STAR:
	case TokenKind::SLASH:
		return {4, 5};
	case TokenKind::LEFT_PAREN:
		return {6, 7};
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
	case TokenKind::DOUBLE_EQUAL:
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

std::optional<ASTNode> Parser::parse_block(
	std::function<bool(TokenKind kind)> end_predicate) {
	auto node = new_node<BlockStmt>(ASTNodeKind::BLOCK);
	
	while (true) {
		if (this->tokens.is_eof()) {
			Diagnostic(DiagnosticKind::UNEXPECTED_EOF, this->tokens.prev())
				.emit(std::cout);
			return std::nullopt;
		}

		if (end_predicate(this->tokens.peek().kind)) {
			return ASTNode(&node->kind);
		}

		if (auto stmt = parse_stmt(); !stmt) {
			return std::nullopt;
		} else {
			node->block.push_back(*stmt);
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
			// Parse arguments as `FuncArgument`.
			auto node = new_node<FuncArgument>(ASTNodeKind::FUNC_ARGUMENTS);

			if (!parse_type_annotation<FuncArgument*>(node)) {
				return false;
			}
			buf.push_back(std::move(ASTNode(&node->kind)));
		} else {
			// Parse arguments as expression.
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
