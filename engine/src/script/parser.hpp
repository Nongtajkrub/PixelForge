#pragma once

#include "../core/bump_arena.hpp"
#include "location.hpp"
#include "token.hpp"
#include "symbol_table.hpp"
#include "diagnostic.hpp"
#include "ast.hpp"
#include "pattern.hpp"

#include <cassert>
#include <cstddef>
#include <functional>
#include <optional>
#include <type_traits>
#include <vector>
#include <ostream>

namespace scr {

using namespace core;

class Parser {
private:
	// Arena allocator for nodes.
	BumpArena& arena;

	TokenStream tokens;

	SymbolTable& symbols;

	// The stream to output error to.
	std::ostream& err_stream;

	// Abstract Syntax Tree separated into each statements.
	std::vector<ASTNode> ast;

public:
	Parser(
		const std::span<Token> tokens,
		SymbolTable& symbols, BumpArena& arena, std::ostream& err_stream) :
		tokens(tokens, err_stream),
		symbols(symbols),
		arena(arena),
		err_stream(err_stream)
	{ }

	bool parse();

	inline const std::vector<ASTNode>& get_ast() {
		return this->ast;
	}

	inline const ASTNode get_ast(size_t line) {
		return this->ast[line];
	}

private:
	std::optional<ASTNode> parse_block(
		std::function<bool(TokenKind kind)> end_predicate);

	std::optional<ASTNode> parse_stmt();

	std::optional<ASTNode> parse_var_declaration_stmt();
	std::optional<ASTNode> parse_func_declaration_stmt();
	std::optional<ASTNode> parse_if_stmt();
	std::optional<ASTNode> parse_for_stmt();
	std::optional<ASTNode> parse_return_stmt();
	std::optional<ASTNode> parse_jump_stmt();
	std::optional<ASTNode> parse_cmd_stmt();
	std::optional<ASTNode> parse_directive();

	std::optional<ASTNode> parse_expr(TokenKind terminator);
	std::optional<ASTNode> pratt_nud();
	std::optional<ASTNode> pratt_led(Token op, ASTNode left, u8 min_bp);
	std::optional<ASTNode> pratt_parser(u8 min_bp = 0);

	std::optional<ASTNode> parse_atomic(ASTNodeKind kind);

	// Usually parse arguments as `FuncArgument` but can also parse as expression.
	bool parse_func_args(std::vector<ASTNode>& buf, IdenAttr& func_attr);
	bool parse_func_call_args(
		std::vector<ASTNode>& buf,
		std::vector<TokenKind> arg_types, TokenKind terminator);

	// Resolve data type of expression, take source location to emit errors.
	// Optional ltype, typically passed internally during binary expr parsing.
	std::optional<TokenKind> resolve_expr_type(ASTNode expr, Location err_loc);

	inline void emit(DiagnosticKind kind, const Token& token) {
		Diagnostic(kind, token).emit(this->err_stream);
	}

	inline void emit(DiagnosticKind kind, Location loc) {
		Diagnostic(kind, loc).emit(this->err_stream);
	}

	// Parse `IDENTIFIER : "type"`, only work for variable declaration and
	// function arguments. Return the type if success.
	template<typename T>
	requires (std::is_same_v<T, VarDeclarationStmt*> 
		|| std::is_same_v<T, FuncArgument*>)
	std::optional<TokenKind> parse_type_annotation(T node) {
		// Ensure correct type annotation syntax.
		if (!Pattern<
				TokenStream,
				TokenKind::IDENTIFIER,
				TokenKind::COLON>
					::match_peek(this->tokens, this->err_stream)) {
			return std::nullopt;
		}

		const auto id =
			this->symbols.intern_iden(*(this->tokens.advance().lexeme)); 

		this->tokens.advance(); // Skip colon (':').
		
		// Ensure parsed type is a value type.
		if (!token_is_value_type(this->tokens.peek().kind)) {
			emit(DiagnosticKind::EXPECTED_VALUE_TYPE, this->tokens.peek());
			return std::nullopt;
		}
		const auto& type_token = this->tokens.advance();
		node->type = new_primary_node(ASTNodeKind::TYPE, type_token); 

		node->identifier =
			new_identifier_node(
				id,
				&this->symbols.new_identifier(
					id, IdenAttr(IdenKind::VAR, type_token.kind)));

		return type_token.kind;
	}

	inline bool ensure_iden_exist(IdentifierId id) {
		if (!this->symbols.contains(id)) {
			emit(DiagnosticKind::UNKNOWN_IDENTIFIER, this->tokens.peek()); 
			return false;
		}
		return true;
	}

	inline bool ensure_iden_type(IdentifierId id, TokenKind type) {
		assert(token_is_type(type));

		if (const auto attr = this->symbols.lookup(id); attr) {
			if ((*attr).get().type != type) {
				emit(DiagnosticKind::TYPE_ERROR, this->tokens.advance());
				return false;
			}
		} else {
			emit(DiagnosticKind::UNKNOWN_IDENTIFIER, this->tokens.advance());
			return false;
		}
		return true;
	}

	// Take source location to emit errors.
	inline bool ensure_type_same(TokenKind t1, TokenKind t2, Location loc) {
		assert(token_is_type(t1) && token_is_type(t2));

		if (t1 != t2) {
			emit(resolve_diag_expect_kind(t2), loc);
			return false;
		}
		return true;
	}

	template <typename  T>
	inline T* new_node(ASTNodeKind kind) {
		auto node = this->arena.alloc<T>();
		node->kind = kind;
		return node;
	}

	inline ASTNode new_primary_node(ASTNodeKind kind, const Token& token) {
		auto node = this->arena.alloc<PrimaryExpr>();
		node->kind = kind;
		node->token = token;
		return ASTNode(&node->kind);
	}

	inline ASTNode new_identifier_node(
		const std::string& symbol, IdenAttr* attr) {
		auto node = this->arena.alloc<IdentifierExpr>();
		node->kind = ASTNodeKind::IDENTIFIER;
		node->id = this->symbols.intern_iden(symbol);
		node->attr = attr;
		return ASTNode(&node->kind);
	}

	inline ASTNode new_identifier_node(IdentifierId id, IdenAttr* attr) {
		auto node = this->arena.alloc<IdentifierExpr>();
		node->kind = ASTNodeKind::IDENTIFIER;
		node->id = id;
		node->attr = attr;
		return ASTNode(&node->kind);
	}
};

} // namespace scr
