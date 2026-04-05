#include "../core/log.hpp"
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

std::optional<ASTNode> Parser::parse_stmt() {
	switch (this->tokens.peek().kind) {
	case TokenKind::LET:
		return parse_var_declaration_stmt();
	case TokenKind::FUNC:
		return parse_func_declaration_stmt();
	case TokenKind::IF:
		return parse_if_stmt();
	case TokenKind::FOR:
		return parse_for_stmt();
	case TokenKind::RETURN:
		return parse_return_stmt();
	case TokenKind::CONTINUE:
	case TokenKind::BREAK:
		return parse_jump_stmt();
	case TokenKind::COMMAND:
		return parse_cmd_stmt();
	case TokenKind::DIRECT_SPRITE:
	case TokenKind::DIRECT_USE:
	case TokenKind::DIRECT_UPDATE:
	case TokenKind::DIRECT_COLLIDE:
		return parse_directive();
	case TokenKind::IDENTIFIER:
		return parse_expr(TokenKind::SEMICOLON);
	case TokenKind::PASS:
		return parse_atomic(ASTNodeKind::NOP);
	default:
		emit(DiagnosticKind::UNEXPECTED_TOKEN, this->tokens.advance());
		return std::nullopt;
	}
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
		emit(DiagnosticKind::UNEXPECTED_TOKEN, this->tokens.prev());
		return std::nullopt;
	}
}

std::optional<ASTNode> Parser::parse_func_declaration_stmt() {
	auto node = new_node<FuncDeclarationStmt>(ASTNodeKind::FUNC_DECLARATION);

	if (this->symbols.this_scope() != ScopeKind::GLOBAL) {
		emit(DiagnosticKind::FUNC_DECL_NOT_ALLOW, this->tokens.peek());
		return std::nullopt;
	}

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
		emit(DiagnosticKind::EXPECTED_TYPE, this->tokens.peek());
		return std::nullopt;
	}
	const auto& type_token = this->tokens.advance();

	// Create a new identifier symbol for the function.
	auto& attr =
		this->symbols.new_identifier(
			id, IdenAttr(IdenKind::FUNC, type_token.kind));
	node->identifier = new_identifier_node(id, &attr);
	node->type = new_primary_node(ASTNodeKind::TYPE, type_token);


	// Push scope here to include function arguments in the scope as well.
	this->symbols.enter_scope(ScopeKind::FUNC);

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
			[](auto kind) -> bool { return kind == TokenKind::END; }));

	// Ignore the end keyword and go out of function scope.
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

	this->symbols.enter_scope(ScopeKind::IF);
	
	// parse then branch.
	OPT_ASSIGN_OR_RETURN(
		node->then_branch,
		parse_block(
			[](auto kind) -> bool {
				return kind == TokenKind::END || kind == TokenKind::ELSE; }));

	// parse else branch if exist.
	if (this->tokens.match_kind(TokenKind::ELSE)) {
		if (!this->tokens.expect(TokenKind::SEMICOLON)) {
			return std::nullopt;
		}

		OPT_ASSIGN_OR_RETURN(
			node->else_branch,
			parse_block(
				[](auto kind) -> bool { return kind == TokenKind::END; }));
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

std::optional<ASTNode> Parser::parse_for_stmt() {
	auto node = new_node<ForLoopStmt>(ASTNodeKind::FOR_LOOP);

	// Skip for keyword and enter loop scope.
	this->tokens.advance();
	this->symbols.enter_scope(ScopeKind::LOOP);

	switch (this->tokens.peek().kind) {
	case TokenKind::UNDERSCORE:
		node->it = std::nullopt; 
		this->tokens.advance();
		break;
	case TokenKind::IDENTIFIER: {
		const auto id =
			this->symbols.intern_iden(*(this->tokens.advance().lexeme));

		node->it = 
			new_identifier_node(
				id,
				&this->symbols.new_identifier(
					id, IdenAttr(IdenKind::VAR, TokenKind::INT_T)));

		break;
	}
	default:
		emit(DiagnosticKind::UNEXPECTED_TOKEN, this->tokens.peek());
		return std::nullopt;
	}

	const auto range_expr_start = this->tokens.peek().location;
	OPT_ASSIGN_OR_RETURN(node->range, parse_expr(TokenKind::SEMICOLON));

	if (*node->range.adr != ASTNodeKind::RANGE) {
		emit(DiagnosticKind::EXPECTED_RANGE_EXPR, range_expr_start);
		return std::nullopt;
	}
	
	// Parse loop code block.
	OPT_ASSIGN_OR_RETURN(
		node->block,
		parse_block(
			[](auto kind) -> bool { return kind == TokenKind::END; }));

	// Ignore the end keyword and leave loop scope.
	this->tokens.advance();
	this->symbols.leave_scope();

	if (!this->tokens.expect(TokenKind::SEMICOLON)) {
		return std::nullopt;
	}

	return ASTNode(&node->kind);
}

std::optional<ASTNode> Parser::parse_return_stmt() {
	auto node = new_node<ReturnStmt>(ASTNodeKind::RETURN);

	if (!this->symbols.in_scope(ScopeKind::FUNC)) {
		emit(DiagnosticKind::RETURN_NOT_ALLOW, this->tokens.peek());
		return std::nullopt;
	}

	// Ignore the return keyword.
	this->tokens.advance();

	switch (this->tokens.peek().kind) {
	case TokenKind::SEMICOLON:
		this->tokens.advance(); // Ignore the semicolon.
		node->expr = std::nullopt;
		return ASTNode(&node->kind);
	default:
		OPT_ASSIGN_OR_RETURN(node->expr, parse_expr(TokenKind::SEMICOLON));
		return ASTNode(&node->kind);
	}
}

std::optional<ASTNode> Parser::parse_jump_stmt() {
	if (!this->symbols.in_scope(ScopeKind::LOOP)) {
		emit(DiagnosticKind::JUMP_NOT_ALLOW, this->tokens.peek());
		return std::nullopt;
	}

	switch (this->tokens.peek().kind) {
	case TokenKind::BREAK:
		return parse_atomic(ASTNodeKind::BREAK);
	case TokenKind::CONTINUE:
		return parse_atomic(ASTNodeKind::CONTINUE);
	default:
		LOG_ERR("This statement is not a jump one");
		std::unreachable();
	}
}

std::optional<ASTNode> Parser::parse_cmd_stmt() {
	auto node = new_node<CommandStmt>(ASTNodeKind::COMMAND);

	const std::string& cmd = *this->tokens.peek().lexeme;
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

		if (const auto& attr_ref = this->symbols.lookup(id)) {
			node->target = new_identifier_node(id, &(*attr_ref).get());
		} else {
			emit(DiagnosticKind::UNKNOWN_IDENTIFIER, this->tokens.peek());
			return std::nullopt;
		}

		break;
	}
	case TokenKind::SELF:
		node->target = ASTNode(&(new_node<AtomicNode>(ASTNodeKind::SELF)->kind));
		this->tokens.advance();
		break;
	default:
		emit(DiagnosticKind::UNEXPECTED_TOKEN, this->tokens.advance());
		return std::nullopt;
	}

	if (this->tokens.match_kind(TokenKind::SEMICOLON)) {
		node->operands = {};
		return ASTNode(&node->kind);
	}

	if (!parse_func_call_args(
			node->operands,
			get_command_args(cmd), TokenKind::SEMICOLON)) {
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
	node->identifier =
		new_identifier_node(
			id,
			&this->symbols.new_identifier_global(
				id, IdenAttr(IdenKind::VAR, TokenKind::SPRITE_T))); 

	this->tokens.advance(); // Skip semicolon.

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
	case TokenKind::RANGE_OP:
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
		return 
			new_literal_node(
				this->cpool.intern_const(Const(this->tokens.advance())));
	case TokenKind::TRUE: 
		this->tokens.advance(); // Ignore true keyword.
		return ASTNode(&(new_node<AtomicNode>(ASTNodeKind::TRUE)->kind));
	case TokenKind::FALSE:
		this->tokens.advance(); // Ignore false keyword.
		return ASTNode(&(new_node<AtomicNode>(ASTNodeKind::FALSE)->kind));
	case TokenKind::SELF:
		this->tokens.advance();
		return ASTNode(&(new_node<AtomicNode>(ASTNodeKind::SELF)->kind));
	case TokenKind::IDENTIFIER: { 
		const auto id =
			this->symbols.intern_iden(*(this->tokens.advance().lexeme));

		if (const auto& attr_ref = this->symbols.lookup(id)) {
			return new_identifier_node(id, &(*attr_ref).get());
		} else {
			emit(DiagnosticKind::UNKNOWN_IDENTIFIER, this->tokens.peek());
			return std::nullopt;
		}
	}
	case TokenKind::LEFT_PAREN: 
		// Skip the left paren.
		this->tokens.advance(); 
		return parse_expr(TokenKind::RIGHT_PAREN); 
	default:
		emit(DiagnosticKind::UNEXPECTED_TOKEN, this->tokens.advance());
		return std::nullopt;
	}
}

std::optional<ASTNode> Parser::pratt_led(Token op, ASTNode left, u8 min_bp) {
	switch (op.kind) {
	// Parse binary expression.
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
	// Parse call expression.
	case TokenKind::LEFT_PAREN: {
		if (*left.adr != ASTNodeKind::IDENTIFIER) {
			emit(DiagnosticKind::EXPECTED_IDENTIFIER, op);
			return std::nullopt;
		}

		auto node = new_node<CallExpr>(ASTNodeKind::CALL);
		node->identifier = left;

		if (auto func_attr_ref =
				this->symbols.lookup(
					reinterpret_cast<const IdentifierExpr*>(left.adr)->id)) {
			if ((*func_attr_ref).get().kind != IdenKind::FUNC) {
				emit(DiagnosticKind::EXPECTED_FUNCTION, op);
				return std::nullopt;
			}

			// Added for readability.
			const std::optional<std::vector<TokenKind>>& args_types =
				(*func_attr_ref).get().arg_types;

			if (!parse_func_call_args(
					node->args, *args_types, TokenKind::RIGHT_PAREN)) {
				return std::nullopt;
			}
		} else {
			return std::nullopt;
		}

		return ASTNode(&node->kind);
	}
	// Parse assignment expression.
	case TokenKind::EQUAL: {
		if (*left.adr != ASTNodeKind::IDENTIFIER) {
			emit(DiagnosticKind::EXPECTED_IDENTIFIER, op);
			return std::nullopt;
		}

		auto node = new_node<AssignStmt>(ASTNodeKind::ASSIGN);

		node->identifier = left;
		OPT_ASSIGN_OR_RETURN(node->expr, pratt_parser());

		// Type checking
		const auto id = reinterpret_cast<const IdentifierExpr*>(left.adr)->id;

		const auto left_attr_ref = this->symbols.lookup(id);
		if (!left_attr_ref) {
			LOG_ERR("Unkown identifier, ensure pratt parser checked existence.");
			std::unreachable();
		}

		if (const auto expr_type = resolve_expr_type(node->expr, op.location)) {
			if (!ensure_type_same(
					(*left_attr_ref).get().type, *expr_type, op.location)) {
				return std::nullopt;
			}
		} else {
			return std::nullopt;
		}

		return ASTNode(&node->kind);
	}
	// Parse dot notation expression.
	case TokenKind::DOT: {
		if (*left.adr != ASTNodeKind::IDENTIFIER 
				&& *left.adr != ASTNodeKind::SELF) {
			emit(DiagnosticKind::EXPECTED_IDENTIFIER, op);
			return std::nullopt;
		 }
 
		auto node = new_node<DotExpr>(ASTNodeKind::DOT);

		node->object = left;

		if (!token_is_property(this->tokens.peek())) {
			emit(DiagnosticKind::EXPECTED_PROPERTY, this->tokens.advance());
			return std::nullopt;
		}

		node->property = (*(this->tokens.advance().lexeme)).front();

		return ASTNode(&node->kind);
	}
	// Parse range expression.
	case TokenKind::RANGE_OP: {
		auto node = new_node<RangeExpr>(ASTNodeKind::RANGE);

		node->begin = left;
		OPT_ASSIGN_OR_RETURN(node->end, pratt_parser());

		// Ensure both begin and end are integer.
		const auto btype = resolve_expr_type(node->begin, op.location);
		const auto etype = resolve_expr_type(node->end, op.location);
		if (!btype || !etype) {
			return std::nullopt;
		}
		if (!ensure_type_same(*btype, TokenKind::INT_T, op.location) 
				|| !ensure_type_same(*etype, TokenKind::INT_T, op.location)) {
			return std::nullopt;
		}

		switch (this->tokens.peek().kind) {
		case TokenKind::COMMA:
			this->tokens.advance(); // Skip the comma.
			OPT_ASSIGN_OR_RETURN(node->step, pratt_parser());
			break;
		case TokenKind::SEMICOLON:
			node->step = std::nullopt;
			break;
		default:
			emit(DiagnosticKind::UNEXPECTED_TOKEN, this->tokens.peek());
			return std::nullopt;
		}

		return ASTNode(&node->kind);
	}
	default:
		emit(DiagnosticKind::UNEXPECTED_TOKEN, this->tokens.advance());
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

std::optional<ASTNode> Parser::parse_atomic(ASTNodeKind kind) {
	auto node = new_node<AtomicNode>(kind);
	this->tokens.advance();
	if (!this->tokens.expect(TokenKind::SEMICOLON)) {
		return std::nullopt;
	}
	return ASTNode(&node->kind);
}

bool Parser::parse_func_args(std::vector<ASTNode>& buf, IdenAttr& func_attr) {
	assert(func_attr.arg_types && func_attr.kind == IdenKind::FUNC);

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
			(*func_attr.arg_types).push_back(*arg_type);
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
			emit(DiagnosticKind::EXPECTED_COMMA, this->tokens.peek());
			return false;
		}
	}

	return false;
}

bool Parser::parse_func_call_args(
	std::vector<ASTNode>& buf,
	std::vector<TokenKind> arg_types, TokenKind terminator) {
	auto arg_it = arg_types.begin();

	while (!this->tokens.is_eof()) {
		if (this->tokens.match_kind(terminator)) {
			return true;
		}

		// More argument pass in than the definition.
		if (arg_it == arg_types.end()) {
			emit(DiagnosticKind::TO_MANY_ARGUMENTS, this->tokens.peek());
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
		} else if (kind != terminator) {
			emit(DiagnosticKind::EXPECTED_COMMA, this->tokens.peek());
			return false;
		}
	}

	return false;
}

std::optional<TokenKind> Parser::resolve_expr_type(
	ASTNode expr, Location err_loc) {
	switch (*expr.adr) {
	case ASTNodeKind::FALSE:
	case ASTNodeKind::TRUE:
		return TokenKind::BOOL_T;
	case ASTNodeKind::LITERAL: {
		return 
			this->cpool.get(
				reinterpret_cast<const LiteralExpr*>(expr.adr)->index).type; 
	}
	case ASTNodeKind::DOT:
		return 
			property_to_type(
				reinterpret_cast<const DotExpr*>(expr.adr)->property);
	case ASTNodeKind::CALL: 
		return 
			resolve_expr_type(
				reinterpret_cast<const CallExpr*>(expr.adr)->identifier, err_loc);
	case ASTNodeKind::IDENTIFIER: {
		const auto id = reinterpret_cast<const IdentifierExpr*>(expr.adr)->id;

		if (const auto attr_ref = this->symbols.lookup(id)) {
			return (*attr_ref).get().type;
		}

		// Identifier dont exist.
		emit(DiagnosticKind::UNKNOWN_IDENTIFIER, err_loc);
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
	emit(DiagnosticKind::TYPE_ERROR, err_loc);
	return std::nullopt;
}

} // namespace scr;
