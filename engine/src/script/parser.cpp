#include "../core/io/log.hpp"
#include "diagnostic.hpp"
#include "location.hpp"
#include "symbol_table.hpp"
#include "token.hpp"
#include "parser.hpp"
#include "ast.hpp"
#include "pattern.hpp"

#include <cassert>
#include <cstddef>
#include <optional>
#include <tuple>
#include <utility>
#include <vector>

namespace scr {

bool Parser::parse() {
	while (!this->tokens.is_eof()) {
		if (this->tokens.peek().skip) {
			this->tokens.advance();
			continue;
		}

		// Recursively parse the code.
		if (auto node = parse_stmt(); node) {
			this->ast.push_back(*node);
		} else {
			return false;
		}
	}
	return true;
}

std::optional<TokenKind> Parser::resolve_expr_type(
	ASTNode expr, Location err_loc) {
	switch (*expr.adr) {
	case ASTNodeKind::LITERAL: {
		const auto type =
			token_kind_as_type(
				reinterpret_cast<const PrimaryExpr*>(expr.adr)->token.kind);

		return type;
	}
	case ASTNodeKind::IDENTIFIER: {
		const auto id = reinterpret_cast<const IdentifierExpr*>(expr.adr)->id;

		if (const auto attr = this->symbols.lookup(id)) {
			return (*attr).get().type;
		}

		// Identifier dont exist.
		Diagnostic(DiagnosticKind::UNKNOWN_IDENTIFIER, err_loc)
			.emit(this->err_stream);
		return std::nullopt;
	}
	case ASTNodeKind::BINARY: {
		const auto node = reinterpret_cast<const BinaryExpr*>(expr.adr);

		const auto ltype = resolve_expr_type(node->left, err_loc);
		if (!ltype) {
			return std::nullopt;
		}

		const auto rtype = resolve_expr_type(node->right, err_loc);
		if (!rtype) {
			return std::nullopt;
		}

		if (rtype != ltype) {
			goto type_err;
		}

		if (token_is_comparison_operator(node->op.kind)) {
			return TokenKind::BOOL_T;
		}

		return rtype;
	} 
	default:
		goto type_err;
	}

type_err:
	Diagnostic(DiagnosticKind::TYPE_ERROR, err_loc)
		.emit(this->err_stream);
	return std::nullopt;
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
		return parse_expr(TokenKind::SEMICOLON);
	case TokenKind::COMMAND:
		return parse_cmd_stmt();
	default:
		const auto& token = this->tokens.peek();

		if (token_is_directive(token.kind)) {
			return parse_directive();
		}

		Diagnostic(DiagnosticKind::UNEXPECTED_TOKEN, this->tokens.advance())
			.emit(this->err_stream);
		return std::nullopt;
	}
}

std::optional<ASTNode> Parser::parse_nop() {
	auto node = new_node<AtomicNode>(ASTNodeKind::NOP);
	this->tokens.advance();
	if (!this->tokens.expect(TokenKind::SEMICOLON)) {
		return std::nullopt;
	}
	return ASTNode(&node->kind);
}

std::optional<ASTNode> Parser::parse_directive() {
	auto node = new_node<DirectiveStmt>(ASTNodeKind::DIRECTIVE);

	node->directive =  
		new_primary_node(ASTNodeKind::KEYWORD, this->tokens.advance());

	// Ensure correct pattern.
	if (!Pattern<
			TokenStream,
			TokenKind::IDENTIFIER,
			TokenKind::SEMICOLON>::match_peek(this->tokens, this->err_stream)) {
		return std::nullopt;
	}

	const auto id = this->symbols.intern_iden(*(this->tokens.advance().lexeme));
	node->identifier = new_identifier_node(id);
	this->symbols.new_identifier_global(
		id, IdenAttr(IdenKind::VAR, TokenKind::SPRITE_T)); 

	this->tokens.advance(); // Skip semicolon.

	return ASTNode(&node->kind);
}

std::optional<ASTNode> Parser::parse_var_declaration_stmt() {
	auto node = new_node<VarDeclarationStmt>(ASTNodeKind::VAR_DECLARATION);

	// Ignore "let" keyword.
	this->tokens.advance();

	if (!parse_type_annotation<VarDeclarationStmt*>(node)) {
		return std::nullopt;
	}
	const auto iden_type =
		reinterpret_cast<const PrimaryExpr*>(node->type.adr)->token.kind;

	if (!this->tokens.ensure_not_eof()) {
		return std::nullopt;
	}

	// Resolve whether the declaration statment initialize anything.
	switch (this->tokens.advance().kind) {
	case TokenKind::SEMICOLON:
		node->init = std::nullopt;
		return ASTNode(&node->kind);
	case TokenKind::EQUAL: {
		const auto eq_loc = this->tokens.prev().location;

		if (node->init = parse_expr(TokenKind::SEMICOLON); !node->init) {
			return std::nullopt;
		}

		if (const auto expr_type = resolve_expr_type(*node->init, eq_loc)) {
			if (!ensure_type_same(iden_type, *expr_type, eq_loc)) {
				return std::nullopt;
			}
		} else {
			return std::nullopt;
		}

		return ASTNode(&node->kind);
	}
	default:
		Diagnostic(DiagnosticKind::UNEXPECTED_TOKEN, this->tokens.prev())
			.emit(this->err_stream);
		return std::nullopt;
	}
}

std::optional<ASTNode> Parser::parse_func_declaration_stmt() {
	auto node = new_node<FuncDeclarationStmt>(ASTNodeKind::FUNC_DECLARATION);

	// Ignore "func" keyword.
	this->tokens.advance();

	// Ensure an identifier follow func.
	if (!this->tokens.expect_peek(TokenKind::IDENTIFIER)) {
		return std::nullopt;
	}
	const auto id = this->symbols.intern_iden(*(this->tokens.advance()).lexeme);

	if (!this->tokens.expect(TokenKind::ARROW)) {
		return std::nullopt;
	}

	if (!token_is_type(this->tokens.peek().kind)) {
		Diagnostic(DiagnosticKind::EXPECTED_TYPE, this->tokens.peek())
			.emit(this->err_stream);
		return std::nullopt;
	}
	const auto& type_token = this->tokens.advance();
	node->identifier = new_identifier_node(id);
	node->type = new_primary_node(ASTNodeKind::TYPE, type_token);

	// Create a new identifier symbol for the function.
	auto& attr =
		this->symbols.new_identifier(
			id, IdenAttr(IdenKind::FUNC, type_token.kind));

	// Push scope here to include function arguments in the scope as well.
	this->symbols.enter_scope();

	if (!parse_func_args(node->args, attr)) {
		return std::nullopt;
	}
	if (!this->tokens.expect(TokenKind::SEMICOLON)) {
		return std::nullopt;
	}

	// Parse function body.
	OPT_ASSIGN_OR_RETURN(
		node->body,
		parse_block(
			[](auto kind) -> bool { return kind == TokenKind::ENDFUNC; }));

	// Ignore the "endfunc" keyword and go out of function scope.
	this->tokens.advance();
	this->symbols.leave_scope();

	if (!this->tokens.expect(TokenKind::SEMICOLON)) {
		return std::nullopt;
	}

	return ASTNode(&node->kind);
}

std::optional<ASTNode> Parser::parse_if_stmt() {
	auto node = new_node<IfStmt>(ASTNodeKind::IF);

	// Ignore "if" keyword.
	this->tokens.advance();

	OPT_ASSIGN_OR_RETURN(node->expr, parse_expr(TokenKind::SEMICOLON));

	this->symbols.enter_scope();
	
	// parse then branch.
	OPT_ASSIGN_OR_RETURN(
		node->then_branch,
		parse_block(
			[](auto kind) -> bool {
				return kind == TokenKind::ENDIF || kind == TokenKind::ELSE; }));

	// parse else branch if exist.
	if (this->tokens.match_kind(TokenKind::ELSE)) {
		if (!this->tokens.expect(TokenKind::SEMICOLON)) {
			return std::nullopt;
		}

		OPT_ASSIGN_OR_RETURN(
			node->else_branch,
			parse_block(
				[](auto kind) -> bool { return kind == TokenKind::ENDIF; }));
	} else {
		node->else_branch = std::nullopt;
	}

	// Ignore "endif" keyword and go out of if scope.
	this->tokens.advance();
	this->symbols.leave_scope();

	if (!this->tokens.expect(TokenKind::SEMICOLON)) {
		return std::nullopt;
	}

	return ASTNode(&node->kind);
}

std::optional<ASTNode> Parser::parse_cmd_stmt() {
	auto node = new_node<CommandStmt>(ASTNodeKind::COMMAND);

	node->command =
		new_primary_node(ASTNodeKind::KEYWORD, this->tokens.advance());

	if (!this->tokens.ensure_not_eof()) {
		return std::nullopt;
	}

	// Expect both identifier and the self keyword.
	switch (this->tokens.peek().kind) {
	case TokenKind::IDENTIFIER: {
		const auto id = 
			this->symbols.intern_iden(*(this->tokens.advance().lexeme));
		if (!ensure_iden_exist(id)) {
			return std::nullopt;
		}
		node->target = new_identifier_node(id);
		break;
	}
	case TokenKind::SELF:
		node->target = ASTNode(&(new_node<AtomicNode>(ASTNodeKind::SELF)->kind));
		this->tokens.advance();
		break;
	default:
		Diagnostic(DiagnosticKind::UNEXPECTED_TOKEN, this->tokens.advance())
			.emit(this->err_stream);
		return std::nullopt;
	}

	if (this->tokens.match_kind(TokenKind::SEMICOLON)) {
		node->operands = {};
		return ASTNode(&node->kind);
	}

	// TODO: Handle command argument parsing.
	/*
	if (!parse_func_args(node->operands, TokenKind::SEMICOLON, true)) {
		return std::nullopt;
	}
	*/
	TODO();

	return ASTNode(&node->kind);
}

// Will consume the expression terminator token.
std::optional<ASTNode> Parser::parse_expr(TokenKind terminator) {
	auto node = pratt_parser();
	if (!node) {
		return std::nullopt;
	}

	// Ensure correct expression terminaltor token and consume it.
	if (!this->tokens.expect(terminator)) {
		return std::nullopt;
	}

	return node;
}

static std::tuple<u8, u8> resolve_binding_power(TokenKind kind) {
	switch (kind) {
	case TokenKind::EQUAL: 
		return {2, 1};
	case TokenKind::AND:
	case TokenKind::OR:
		return {2, 3};
	case TokenKind::BANG_EQUAL:
	case TokenKind::DOUBLE_EQUAL:
	case TokenKind::GREATER:
	case TokenKind::GREATER_EQUAL:
	case TokenKind::LESS:
	case TokenKind::LESS_EQUAL:
		return {4, 5};
	case TokenKind::PLUS:
	case TokenKind::MINUS:
		return {7, 8};
	case TokenKind::STAR:
	case TokenKind::SLASH:
		return {9, 10};
	case TokenKind::LEFT_PAREN:
		return {11, 12};
	case TokenKind::DOT:
		return {14, 13};
	default:
		return {0, 0};
	}
}

std::optional<ASTNode> Parser::pratt_nud() {
	switch (this->tokens.peek().kind) {
	case TokenKind::INTEGER:
	case TokenKind::FLOAT:
	case TokenKind::STRING:
		return new_primary_node(ASTNodeKind::LITERAL, this->tokens.advance());
	case TokenKind::SELF:
		this->tokens.advance();
		return ASTNode(&(new_node<AtomicNode>(ASTNodeKind::SELF)->kind));
	case TokenKind::IDENTIFIER: { 
		const auto id  =
			this->symbols.intern_iden(*(this->tokens.advance().lexeme));
		if (!ensure_iden_exist(id)) {
			return std::nullopt;
		}
		return new_identifier_node(id);
	}
	case TokenKind::LEFT_PAREN: 
		// Skip the left paren.
		this->tokens.advance(); 
		return parse_expr(TokenKind::RIGHT_PAREN); 
	default:
		Diagnostic(DiagnosticKind::UNEXPECTED_TOKEN, this->tokens.advance())
			.emit(this->err_stream);
		return std::nullopt;
	}
}

std::optional<ASTNode> Parser::pratt_led(Token op, ASTNode left, u8 min_bp) {
	switch (op.kind) {
	case TokenKind::DOUBLE_EQUAL:
	case TokenKind::BANG_EQUAL:
	case TokenKind::GREATER:
	case TokenKind::GREATER_EQUAL:
	case TokenKind::LESS:
	case TokenKind::LESS_EQUAL:
	case TokenKind::AND:
	case TokenKind::OR:
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
			Diagnostic(DiagnosticKind::EXPECTED_IDENTIFIER, op)
				.emit(this->err_stream);
			return std::nullopt;
		}

		auto node = new_node<CallExpr>(ASTNodeKind::CALL);
		node->identifier = left;

		if (auto func_attr =
				this->symbols.lookup(
					reinterpret_cast<const IdentifierExpr*>(left.adr)->id)) {
			if (!parse_func_call_args(*func_attr, node->args)) {
				return std::nullopt;
			}
		} else {
			return std::nullopt;
		}

		return ASTNode(&node->kind);
	}
	case TokenKind::EQUAL: {
		if (*left.adr != ASTNodeKind::IDENTIFIER) {
			Diagnostic(DiagnosticKind::EXPECTED_IDENTIFIER, op)
				.emit(this->err_stream);
			return std::nullopt;
		}

		auto node = new_node<AssignStmt>(ASTNodeKind::ASSIGN);

		node->identifier = left;
		OPT_ASSIGN_OR_RETURN(node->expr, pratt_parser());

		// Type checking
		const auto id = reinterpret_cast<const IdentifierExpr*>(left.adr)->id;

		const auto left_attr = this->symbols.lookup(id);
		if (!left_attr) {
			LOG_ERR("Unkown identifier, ensure pratt parser checked existence.");
			std::unreachable();
		}

		if (const auto expr_type = resolve_expr_type(node->expr, op.location)) {
			if (!ensure_type_same(
					(*left_attr).get().type, *expr_type, op.location)) {
				return std::nullopt;
			}
		} else {
			return std::nullopt;
		}

		return ASTNode(&node->kind);
	}
	case TokenKind::DOT: {
		if (*left.adr != ASTNodeKind::IDENTIFIER 
				&& *left.adr != ASTNodeKind::SELF) {
			Diagnostic(DiagnosticKind::EXPECTED_IDENTIFIER, op)
				.emit(this->err_stream);
			return std::nullopt;
		 }
 
		auto node = new_node<DotExpr>(ASTNodeKind::DOT);

		node->object = left;

		if (!token_is_property(this->tokens.peek())) {
			Diagnostic(DiagnosticKind::EXPECTED_PROPERTY, this->tokens.advance())
				.emit(this->err_stream);
			return std::nullopt;
		}

		node->property = (*(this->tokens.advance().lexeme)).front();

		return ASTNode(&node->kind);
	}
	default:
		Diagnostic(DiagnosticKind::UNEXPECTED_TOKEN, this->tokens.advance())
			.emit(this->err_stream);
		return std::nullopt;
	}
}

std::optional<ASTNode> Parser::pratt_parser(u8 min_bp) {
	auto left = pratt_nud();
	if (!left) {
		return std::nullopt;
	}

	while (this->tokens.ensure_not_eof()) {
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
	
	while (true && this->tokens.ensure_not_eof()) {
		if (end_predicate(this->tokens.peek().kind)) {
			return ASTNode(&node->kind);
		}

		if (auto stmt = parse_stmt(); !stmt) {
			return std::nullopt;
		} else {
			node->block.push_back(*stmt);
		}
	}

	return std::nullopt;
}

bool Parser::parse_func_args(std::vector<ASTNode>& buf, IdenAttr& func_attr) {
	assert(func_attr.arg_type_list && func_attr.kind == IdenKind::FUNC);

	if (!this->tokens.expect(TokenKind::LEFT_PAREN)) {
		return false;
	}

	while (this->tokens.ensure_not_eof()) {
		if (this->tokens.match_kind(TokenKind::RIGHT_PAREN)) {
			return true;
		}

		// Parse arguments as `FuncArgument`.
		auto node = new_node<FuncArgument>(ASTNodeKind::FUNC_ARGUMENTS);

		if (const auto arg_type = parse_type_annotation<FuncArgument*>(node)) {
			buf.push_back(ASTNode(&node->kind));
			(*func_attr.arg_type_list).push_back(*arg_type);
		} else {
			return false;
		}

		if (!this->tokens.ensure_not_eof()) {
			return false;
		}

		// RIGHT_PAREN will be handle in the next iteration of the loop.
		if (auto kind = this->tokens.peek().kind; kind == TokenKind::COMMA) { 
			this->tokens.advance();
		} else if (kind != TokenKind::RIGHT_PAREN) {
			Diagnostic(DiagnosticKind::EXPECTED_COMMA, this->tokens.peek())
				.emit(this->err_stream);
			return false;
		}
	}

	return false;
}

bool Parser::parse_func_call_args(
	IdenAttr& func_attr , std::vector<ASTNode>& buf) {
	assert(func_attr.arg_type_list && func_attr.kind == IdenKind::FUNC);
	auto arg_it = (*func_attr.arg_type_list).begin();

	while (!this->tokens.is_eof()) {
		if (this->tokens.match_kind(TokenKind::RIGHT_PAREN)) {
			return true;
		}

		// More argument pass in than the definition.
		if (arg_it == (*func_attr.arg_type_list).end()) {
			Diagnostic(DiagnosticKind::TO_MANY_ARGUMENTS, this->tokens.peek())
				.emit(this->err_stream);
			return false;
		}

		// Type of arguments pass into function much match the definition.
		const auto expr_start_loc = this->tokens.peek().location;

		const auto expr = pratt_parser();
		if (!expr) {
			return false;
		}
		const auto expr_type = resolve_expr_type(*expr, expr_start_loc);
		if (!expr_type) {
			return false;
		}
		if (!ensure_type_same(*expr_type, *arg_it, expr_start_loc)) {
			return false;
		}
		arg_it++;

		buf.push_back(*expr);
		
		// RIGHT_PAREN will be handle in the next iteration of the loop.
		if (auto kind = this->tokens.peek().kind; kind == TokenKind::COMMA) { 
			this->tokens.advance();
		} else if (kind != TokenKind::RIGHT_PAREN) {
			Diagnostic(DiagnosticKind::EXPECTED_COMMA, this->tokens.peek())
				.emit(this->err_stream);
			return false;
		}
	}

	return false;
}

;} // namespace scr
