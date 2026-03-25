#pragma once

#include "../core/arena/bump_arena.hpp"
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

	inline const std::span<ASTNode> get_ast() {
		return this->ast;
	}

	inline const ASTNode get_ast(size_t line) {
		return this->ast[line];
	}

private:
	// Resolve data type of expression, take source location to emit errors.
	std::optional<TokenKind> resolve_expr_type(ASTNode expr, Location loc);

	std::optional<ASTNode> parse_stmt();
	std::optional<ASTNode> parse_nop();
	std::optional<ASTNode> parse_directive();
	std::optional<ASTNode> parse_var_declaration_stmt();
	std::optional<ASTNode> parse_func_declaration_stmt();
	std::optional<ASTNode> parse_if_stmt();
	std::optional<ASTNode> parse_cmd_stmt();
	std::optional<ASTNode> parse_expr(TokenKind terminator);
	std::optional<ASTNode> pratt_nud();
	std::optional<ASTNode> pratt_led(Token op, ASTNode left, u8 min_bp);
	std::optional<ASTNode> pratt_parser(u8 min_bp = 0);
	std::optional<ASTNode> parse_block(
		std::function<bool(TokenKind kind)> end_predicate);

	// Usually parse arguments as `FuncArgument` but can also parse as expression.
	bool parse_func_args(std::vector<ASTNode>& buf, bool as_expr = false);

	// Parse `IDENTIFIER : "type"`, only work for variable declaration and
	// function arguments.
	template<typename T>
	requires (std::is_same_v<T, VarDeclarationStmt*> 
		|| std::is_same_v<T, FuncArgument*>)
	bool parse_type_annotation(T node) {
		// Ensure correct type annotation syntax.
		if (!Pattern<
				TokenStream,
				TokenKind::IDENTIFIER,
				TokenKind::COLON>
					::match_peek(this->tokens, this->err_stream)) {
			return false;
		}

		const auto id =
			this->symbols.intern_iden(*(this->tokens.advance().lexeme)); 

		this->tokens.advance(); // Skip colon (':').
		
		// Ensure parsed type actually exist.
		if (!token_is_type(this->tokens.peek().kind)) {
			Diagnostic(DiagnosticKind::EXPECTED_TYPE, this->tokens.peek())
				.emit(this->err_stream);
			return false;
		}
		const auto& type_token = this->tokens.advance();
		this->symbols.new_identifier(id, IdenAttr(type_token.kind));
		node->identifier = new_identifier_node(id);
		node->type = new_primary_node(ASTNodeKind::TYPE, type_token); 

		return true;
	}

	inline bool ensure_iden_exist(UniversalIdType id) {
		if (!this->symbols.contains(id)) {
			Diagnostic(DiagnosticKind::UNKNOWN_IDENTIFIER, this->tokens.peek()) 
				.emit(this->err_stream);
			return false;
		}
		return true;
	}

	inline bool ensure_iden_type(UniversalIdType id, TokenKind type) {
		assert(token_is_type(type));

		if (const auto attr = this->symbols.lookup(id); attr) {
			if ((*attr).type != type) {
				Diagnostic(DiagnosticKind::TYPE_ERROR, this->tokens.advance())
					.emit(this->err_stream);
				return false;
			}
		} else {
			Diagnostic(DiagnosticKind::UNKNOWN_IDENTIFIER, this->tokens.advance())
				.emit(this->err_stream);
			return false;
		}
		return true;
	}

	// Take source location to emit errors.
	inline bool ensure_type_same(TokenKind t1, TokenKind t2, Location loc) {
		assert(token_is_type(t1) && token_is_type(t2));

		if (t1 != t2) {
			Diagnostic(DiagnosticKind::TYPE_ERROR, loc).emit(this->err_stream);
			return false;
		}
		return true;
	}

	inline ASTNode new_primary_node(ASTNodeKind kind, const Token& token) {
		auto node = this->arena.alloc<PrimaryExpr>();
		node->kind = kind;
		node->token = token;
		return ASTNode(&node->kind);
	}

	inline ASTNode new_identifier_node(const std::string& symbol) {
		auto node = this->arena.alloc<IdentifierExpr>();
		node->kind = ASTNodeKind::IDENTIFIER;
		node->id = this->symbols.intern_iden(symbol);
		return ASTNode(&node->kind);
	}

	inline ASTNode new_identifier_node(UniversalIdType id) {
		auto node = this->arena.alloc<IdentifierExpr>();
		node->kind = ASTNodeKind::IDENTIFIER;
		node->id = id;
		return ASTNode(&node->kind);
	}

	template <typename  T>
	inline T* new_node(ASTNodeKind kind) {
		auto node = this->arena.alloc<T>();
		node->kind = kind;
		return node;
	}
};

} // namespace scr
