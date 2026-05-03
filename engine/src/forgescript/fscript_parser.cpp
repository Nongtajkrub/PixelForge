#include "fscript_parser.hpp"

#include "../core/cplusplus/macros.hpp"
#include "../core/c/io/log.h"
#include "fscript_symbol_table.hpp"
#include "fscript_diagnostic.hpp"
#include "fscript_location.hpp"
#include "fscript_pattern.hpp"
#include "fscript_token.hpp"
#include "fscript_ast.hpp"
#include "fscript_specs.h"

#include <optional>
#include <cassert>
#include <cstddef>
#include <vector>
#include <tuple>

namespace scr {

bool Parser::parse() {
	while (!this->tokens.is_eof()) {
		if (this->tokens.peek().skip) {
			this->tokens.advance();
			continue;
		}

		// Recursively parse the code.
		if (auto node = parse_stmt(); node) {
			// Node address being nullptr indeicate success but no node is needed.
			if (!node->adr) continue;

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
		if (end_predicate(this->tokens.peek().kind)) return ASTNode(&node->kind);

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
	case TokenKind::INTERFACE:
		return parse_inter_declaration_stmt();
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
	case TokenKind::DOLLAR_SIGN:
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

	if (!parse_type_annotation<VarDeclarationStmt>(node)) return std::nullopt;
	const auto iden_type =
		reinterpret_cast<const IdentifierExpr*>(node->type.adr)->attr->get_type();

	if (!this->tokens.ensure_not_eof()) {
		return std::nullopt;
	}

	// Resolve whether the declaration statment initialize anything.
	switch (this->tokens.peek().kind) {
	case TokenKind::SEMICOLON:
		this->tokens.advance();
		node->init = std::nullopt;
		return ASTNode(&node->kind);
	case TokenKind::EQUAL: {
		const auto eq_loc = this->tokens.advance().location;

		if (node->init = parse_expr(TokenKind::SEMICOLON); !node->init) 
			return std::nullopt;

		if (!ensure_expr_type(*(node->init), iden_type, eq_loc)) 
			return std::nullopt;

return ASTNode(&node->kind);
	}
	default:
		emit(DiagnosticKind::UNEXPECTED_TOKEN, this->tokens.advance());
		return std::nullopt;
	}
}

std::optional<ASTNode> Parser::parse_func_declaration_stmt() {
	auto node = new_node<FuncDeclarationStmt>(ASTNodeKind::FUNC_DECLARATION);

	if (this->symbols.this_scope() != ScopeKind::GLOBAL) {
		emit(DiagnosticKind::FUNC_DECL_NOT_ALLOW, this->tokens.advance());
		return std::nullopt;
	}

	// Ignore "func" keyword.
	this->tokens.advance();

	// Ensure an identifier follow func.
	if (!this->tokens.expect_peek(TokenKind::IDENTIFIER)) return std::nullopt;
	const auto func_id = this->symbols.intern(*(this->tokens.advance()).lexeme);

	// Create a new identifier symbol for the function (Return type unknown).
	auto attr =
		this->symbols.new_identifier(
			func_id, IdenAttr(nullptr, FuncAttr()));
	node->identifier = new_identifier_node(func_id, attr);

	// Push scope here to include function arguments in the scope as well.
	this->symbols.enter_scope(ScopeKind::FUNC, attr);

	if (!parse_func_args(node->args, attr)) return std::nullopt;

	// Parse return type.
	if (!this->tokens.expect(TokenKind::ARROW)) return std::nullopt;
	if (!this->tokens.expect_peek(TokenKind::IDENTIFIER)) return std::nullopt;

	const auto& type_token = this->tokens.advance();
	const auto type_id = this->symbols.intern(*type_token.lexeme);
	const auto type_attr = lookup_type_iden(type_id, type_token.location);

	if (!type_attr) return std::nullopt;
	node->type = new_identifier_node(type_id, type_attr);
	attr->get_data<FuncAttr>().type = &type_attr->get_data<TypeAttr>();

	if (!this->tokens.expect(TokenKind::SEMICOLON)) return std::nullopt;

	// Parse function body.
	OPT_ASSIGN_OR_RETURN(
		node->body,
		parse_block(
			[](auto kind) -> bool { return kind == TokenKind::END; }));

	// Ensure none void function returns correctly.
	if (type_attr->get_data<TypeAttr>().id != this->symbols.types.ty_void->id) {
		if (!ensure_func_returns(
				reinterpret_cast<const BlockStmt*>(node->body.adr))) {
			emit(DiagnosticKind::FUNC_EXPECTED_RETURN, this->tokens.peek());
			return std::nullopt;
		}
	}

	// Ignore the end keyword and go out of function scope.
	this->tokens.advance();
	this->symbols.leave_scope();

	if (!this->tokens.expect(TokenKind::SEMICOLON)) return std::nullopt;

	return ASTNode(&node->kind);
}

std::optional<ASTNode> Parser::parse_inter_declaration_stmt() {
	if (this->symbols.this_scope() != ScopeKind::GLOBAL) {
		emit(DiagnosticKind::INTERFACE_DECL_NOT_ALLOW, this->tokens.advance());
		return std::nullopt;
	}

	// Ignore the "interface" keyword.
	this->tokens.advance();

	if (!this->tokens.expect_peek(TokenKind::IDENTIFIER)) return std::nullopt;

	const auto type =
		&this->symbols.new_identifier_global(
			this->symbols.intern(*this->tokens.advance().lexeme),
			IdenAttr(TypeAttr()))->get_data<TypeAttr>();
	type->is_value = true;

	if (!this->tokens.expect(TokenKind::SEMICOLON)) return std::nullopt;
	this->symbols.enter_scope(ScopeKind::INTERFACE);

	// Parse properties.
	while (true) {
		if (!Pattern<
				TokenStream,
				TokenKind::IDENTIFIER,
				TokenKind::COLON,
				TokenKind::IDENTIFIER>
					::match_peek(this->tokens, this->err_stream)) {
			return std::nullopt;
		}

		const auto id = this->symbols.intern(*this->tokens.advance().lexeme);
		
		// Ensure property is not being redeclare.
		if (type->has_prop(id)) {
			emit(DiagnosticKind::VARIABLE_REDECLARATION, this->tokens.prev());
			return std::nullopt;
		}

		this->tokens.advance(); // Ignore the ":" token.
		
		const auto type_attr = 
			lookup_type_iden(
				this->symbols.intern(*this->tokens.peek().lexeme),
				this->tokens.peek().location);
		this->tokens.advance(); // Consume type token.
		if (!type_attr) return std::nullopt;

		type->properties.push_back(
			PropAttr(id, &type_attr->get_data<TypeAttr>()));

		if (this->tokens.match_kind(TokenKind::COMMA)) {
			continue;
		} else if (this->tokens.match_kind(TokenKind::END)) {
			break;
		} else {
			emit(DiagnosticKind::UNEXPECTED_TOKEN, this->tokens.advance());
			return std::nullopt;
		}
	}

	if (!this->tokens.expect(TokenKind::SEMICOLON)) return std::nullopt;
	this->symbols.leave_scope();
	return ASTNode(nullptr);
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
		if (!this->tokens.expect(TokenKind::SEMICOLON)) return std::nullopt;

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

	if (!this->tokens.expect(TokenKind::SEMICOLON)) return std::nullopt;

	return ASTNode(&node->kind);
}

std::optional<ASTNode> Parser::parse_for_stmt() {
	auto node = new_node<ForLoopStmt>(ASTNodeKind::FOR_LOOP);

	// Skip for keyword and enter loop scope.
	this->tokens.advance();
	this->symbols.enter_scope(ScopeKind::LOOP);

	// Resolve iterator identifier id.
	IdentifierId id;
	switch (this->tokens.peek().kind) {
	case TokenKind::UNDERSCORE: 
		id = this->symbols.intern(this->identifier_generator.generate());
		this->tokens.advance(); // Ignore underscore.
		break;
	case TokenKind::IDENTIFIER: 
		id = this->symbols.intern(*(this->tokens.advance().lexeme));
		break;
	default:
		emit(DiagnosticKind::UNEXPECTED_TOKEN, this->tokens.peek());
		return std::nullopt;
	}

	node->it = 
		new_identifier_node(
			id,
			this->symbols.new_identifier(
				id, IdenAttr(this->symbols.types.ty_int, VarAttr())));

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

	if (!this->tokens.expect(TokenKind::SEMICOLON)) return std::nullopt;

	return ASTNode(&node->kind);
}

std::optional<ASTNode> Parser::parse_return_stmt() {
	auto node = new_node<ReturnStmt>(ASTNodeKind::RETURN);

	const auto scope_owner = this->symbols.get_scope_owner();

	// Ensure return is allow in current scope.
	if (scope_owner == nullptr || !scope_owner->kind_is<FuncAttr>()) {
		emit(DiagnosticKind::RETURN_NOT_ALLOW, this->tokens.peek());
		return std::nullopt;
	}

	// Ignore the return keyword.
	this->tokens.advance();

	switch (this->tokens.peek().kind) {
	case TokenKind::SEMICOLON:
		// Ensure function is void.
		if (scope_owner->get_type() != nullptr) {
			emit(DiagnosticKind::FUNC_EXPECTED_RETURN, this->tokens.advance());
			return std::nullopt;
		}

		this->tokens.advance(); // Ignore the semicolon.
		node->expr = std::nullopt;
		return ASTNode(&node->kind);
	default:
		const auto expr_loc = this->tokens.peek().location;
		OPT_ASSIGN_OR_RETURN(node->expr, parse_expr(TokenKind::SEMICOLON));

		// Ensure correct return type.
		if (!ensure_expr_type(*(node->expr), scope_owner->get_type(), expr_loc)) 
			return std::nullopt;

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
		BUG("This statement is not a jump one");
		exit(1);
	}
}

std::optional<ASTNode> Parser::parse_cmd_stmt() {
	auto node = new_node<CommandStmt>(ASTNodeKind::COMMAND);

	node->id = command_ids.at(*this->tokens.advance().lexeme);

	if (!this->tokens.expect_peek(TokenKind::IDENTIFIER)) return std::nullopt;

	const auto id = 
		this->symbols.intern(*(this->tokens.advance().lexeme));
	if (const auto& attr = this->symbols.lookup(id)) {
		node->target = new_identifier_node(id, attr);
	} else {
		emit(DiagnosticKind::UNKNOWN_IDENTIFIER, this->tokens.peek());
		return std::nullopt;
	}

	if (this->tokens.match_kind(TokenKind::SEMICOLON)) {
		node->args = {};
		return ASTNode(&node->kind);
	}

	if (!parse_func_call_args(
			node->args, get_command_args(node->id), TokenKind::SEMICOLON))
		return std::nullopt;

	return ASTNode(&node->kind);
}

// Will consume the expression terminator token.
std::optional<ASTNode> Parser::parse_expr(TokenKind terminator) {
	auto node = pratt_parser();
	if (!node) return std::nullopt;

	// Ensure correct expression terminaltor token and consume it.
	if (!this->tokens.expect(terminator)) return std::nullopt;

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
	case TokenKind::INTEGER_LIT:
	case TokenKind::FLOAT_LIT:
	case TokenKind::STRING_LIT:
	case TokenKind::TRUE:
	case TokenKind::FALSE:
		return 
			new_literal_node(this->cpool.intern(Const(this->tokens.advance())));
	case TokenKind::DOLLAR_SIGN: {
		// Ignore dollar sign.
		this->tokens.advance();

		if (!this->tokens.expect_peek(TokenKind::IDENTIFIER)) return std::nullopt;

		const auto id =
			this->symbols.intern(*(this->tokens.advance().lexeme));

		if (const auto attr = this->symbols.lookup_global(id)) {
			return new_identifier_node(id, attr);
		} else {
			emit(DiagnosticKind::UNKNOWN_IDENTIFIER, this->tokens.peek());
			return std::nullopt;
		}
	}
	case TokenKind::IDENTIFIER: { 
		const auto id =
			this->symbols.intern(*(this->tokens.advance().lexeme));

		if (const auto attr = this->symbols.lookup(id)) {
			return new_identifier_node(id, attr);
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

		if (auto func_attr =
				this->symbols.lookup(
					reinterpret_cast<const IdentifierExpr*>(left.adr)->id)) {
			if (!func_attr->kind_is<FuncAttr>()) {
				emit(DiagnosticKind::EXPECTED_FUNCTION, op);
				return std::nullopt;
			}

			if (!parse_func_call_args(
					node->args,
					func_attr->get_data<FuncAttr>().args_types,
					TokenKind::RIGHT_PAREN)) {
				return std::nullopt;
			}
		} else {
			return std::nullopt;
		}

		return ASTNode(&node->kind);
	}
	// Parse assignment expression.
	case TokenKind::EQUAL: {
		if (*left.adr != ASTNodeKind::IDENTIFIER 
				&& *left.adr != ASTNodeKind::DOT) {
			emit(DiagnosticKind::EXPECTED_IDENTIFIER, op);
			return std::nullopt;
		}

		auto node = new_node<AssignStmt>(ASTNodeKind::ASSIGN);

		node->var = left;
		OPT_ASSIGN_OR_RETURN(node->expr, pratt_parser());

		// Ensure both side type are the same.
		const auto var_type = resolve_expr_type(node->var, op.location);
		if (!var_type) return std::nullopt;
		const auto expr_type = resolve_expr_type(node->var, op.location);
		if (!expr_type) return std::nullopt;
		if (var_type != expr_type) return std::nullopt;

		return ASTNode(&node->kind);
	}
	// Parse dot notation expression.
	case TokenKind::DOT: {
		auto node = new_node<DotExpr>(ASTNodeKind::DOT);

		node->object = left;

		if (!this->tokens.expect_peek(TokenKind::IDENTIFIER)) {
			return std::nullopt;
		}

		// Resolve property offset.
		const auto [prop, offset] =
			resolve_expr_type(node->object, op.location)->get_prop_offset(
				this->symbols.intern(*this->tokens.peek().lexeme));
		if (!prop) {
			emit(DiagnosticKind::UNKNOWN_IDENTIFIER, this->tokens.peek());
			return std::nullopt;
		}

		node->prop = prop;
		node->offset = offset;
		this->tokens.advance();

		return ASTNode(&node->kind);
	}
	// Parse range expression.
	case TokenKind::RANGE_OP: {
		auto node = new_node<RangeExpr>(ASTNodeKind::RANGE);

		node->begin = left;
		OPT_ASSIGN_OR_RETURN(node->end, pratt_parser());

		// Ensure both begin and end are integer.
		if (!ensure_expr_type(node->begin, this->symbols.types.ty_int, op.location) 
				|| !ensure_expr_type(
						node->end, this->symbols.types.ty_int, op.location)) 
			return std::nullopt;

		switch (this->tokens.peek().kind) {
		case TokenKind::COMMA: {
			this->tokens.advance(); // Skip the comma.
			const auto expr_loc = this->tokens.peek().location;
			OPT_ASSIGN_OR_RETURN(node->step, pratt_parser());

			if (!ensure_expr_type(
					*node->step, this->symbols.types.ty_int, expr_loc)) 
				return std::nullopt;

			break;
		}
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
	if (!left) return std::nullopt;

	while (this->tokens.ensure_not_eof()) {
		const Token op = this->tokens.peek();

		const auto [lbp, rbp] = resolve_binding_power(op.kind);
		if (lbp <= min_bp) break;

		this->tokens.advance();
		OPT_ASSIGN_OR_RETURN(left, pratt_led(op, *left, rbp));
	}

	return left;
}

std::optional<ASTNode> Parser::parse_atomic(ASTNodeKind kind) {
	auto node = new_node<AtomicNode>(kind);
	this->tokens.advance();
	if (!this->tokens.expect(TokenKind::SEMICOLON)) return std::nullopt;
	return ASTNode(&node->kind);
}

bool Parser::parse_func_args(std::vector<ASTNode>& buf, IdenAttr* func_attr) {
	assert(func_attr->kind_is<FuncAttr>());
	auto& attr = func_attr->get_data<FuncAttr>();

	if (!this->tokens.expect(TokenKind::LEFT_PAREN)) return false;

	while (this->tokens.ensure_not_eof()) {
		if (this->tokens.match_kind(TokenKind::RIGHT_PAREN)) return true;

		// Parse arguments as `FuncArgument`.
		auto node = new_node<FuncArgument>(ASTNodeKind::FUNC_ARGUMENTS);

		if (const auto arg_type = parse_type_annotation<FuncArgument>(node)) {
			buf.push_back(ASTNode(&node->kind));
			attr.args_types.push_back(arg_type);
		} else {
			return false;
		}

		if (!this->tokens.ensure_not_eof()) return false;

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
	const std::vector<TypeAttr*>& arg_types, TokenKind terminator) {
	auto arg_it = arg_types.begin();

	while (!this->tokens.is_eof()) {
		if (this->tokens.match_kind(terminator)) return true;

		// More argument pass in than the definition.
		if (arg_it == arg_types.end()) {
			emit(DiagnosticKind::TO_MANY_ARGUMENTS, this->tokens.peek());
			return false;
		}

		const auto expr_loc = this->tokens.peek().location;

		// Type of arguments pass into function must match the definition.
		if (const auto expr = pratt_parser(); 
				expr 
				&& ensure_expr_type(*expr, *arg_it, expr_loc)) {
			buf.push_back(*expr);
		} else {
			return false;
		}
		arg_it++;
		
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

bool Parser::ensure_func_returns(const BlockStmt* body) {
	for (const auto& node : body->block) {
		if (*node.adr == ASTNodeKind::RETURN) return true;

		if (const auto child_blocks = node.get_child_blocks(); 
				!child_blocks.empty()) {
			// Check whether all child blocks have return statment.
			bool all_block_returns = true;
			for (const auto& block: child_blocks) {
				if (!ensure_func_returns(block)) {
					all_block_returns = false;
					break;
				}
			}
			if (!all_block_returns) {
				continue;
			}

			return true;
		}
	}

	return false;
}

std::vector<TypeAttr*> Parser::get_command_args(command_id_t id) {
	const auto& types = this->symbols.types;

	switch (id) {
	case CID_UP:
	case CID_DOWN:
	case CID_RIGHT:
	case CID_LEFT:
	case CID_WAIT:
		return {types.ty_int};
	case CID_GOTO:
		return {types.ty_int, types.ty_int};
	case CID_SPAWN:
	case CID_DESPAWN:
	case CID_SHOW:
		return {};
	}
}

TypeAttr* Parser::resolve_expr_type(
	ASTNode expr, Location err_loc) {
	switch (*expr.adr) {
	case ASTNodeKind::LITERAL: {
		const auto entry =
			this->cpool.get(
					reinterpret_cast<const LiteralExpr*>(expr.adr)->index); 

		if (entry.data.is<i32>()) {
			return this->symbols.types.ty_int;
		} else if (entry.data.is<f32>()) {
			return this->symbols.types.ty_float;
		} else {
			BUG("Unimplemented const pool type resolving");
			exit(1);
		}
	}
	case ASTNodeKind::DOT: {
		const auto node = reinterpret_cast<const DotExpr*>(expr.adr);

		if (const auto object_type = resolve_expr_type(node->object, err_loc)) {
			if (const auto prop = object_type->get_prop(node->prop->id)) {
				return prop->type;
			} else {
				emit(DiagnosticKind::UNKNOWN_IDENTIFIER, err_loc);
			}
		}

		return nullptr;
	}
	case ASTNodeKind::CALL: 
		return 
			resolve_expr_type(
				reinterpret_cast<const CallExpr*>(expr.adr)->identifier, err_loc);
	case ASTNodeKind::IDENTIFIER: {
		const auto id = reinterpret_cast<const IdentifierExpr*>(expr.adr)->id;

		if (const auto attr = this->symbols.lookup(id)) return attr->get_type();

		// Identifier dont exist.
		emit(DiagnosticKind::UNKNOWN_IDENTIFIER, err_loc);
		return nullptr;
	}
	case ASTNodeKind::BINARY: {
		const auto node = reinterpret_cast<const BinaryExpr*>(expr.adr);

		const auto ltype = resolve_expr_type(node->left, err_loc);
		if (!ltype) return nullptr;

		const auto rtype = resolve_expr_type(node->right, err_loc);
		if (!rtype) return nullptr;

		if (rtype->id != ltype->id) goto type_err;

		if (token_is_comparison_operator(node->op.kind)) 
			return this->symbols.types.ty_bool;

		return rtype;
	} 
	default:
		goto type_err;
	}

type_err:
	emit(DiagnosticKind::TYPE_ERROR, err_loc);
	return nullptr;
}

IdenAttr* Parser::lookup_type_iden(IdentifierId id, Location err_loc) {
	if (const auto attr = this->symbols.lookup_global(id)) {
		if (attr->kind_is<TypeAttr>()) {
			return attr;
		} else {
			emit(DiagnosticKind::EXPECTED_TYPE, this->tokens.peek());
			return nullptr;
		}
	} else {
		emit(DiagnosticKind::UNKNOWN_IDENTIFIER, err_loc);
		return nullptr;
	}
}

// Take source location to emit errors.
bool Parser::ensure_expr_type(
	const ASTNode& expr, TypeAttr* type, Location err_loc) {
	if (const auto expr_type = resolve_expr_type(expr, err_loc)) {
		if (expr_type->id != type->id) {
			emit(DiagnosticKind::TYPE_ERROR, err_loc);
			return false;
		}
	} else {
		return false;
	}

	return true;
}

} // namespace scr;
