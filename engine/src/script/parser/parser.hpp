#pragma once

#include "../../core/arena/bump_arena.hpp"
#include "../common/source_stream.hpp"
#include "../common/token.hpp"
#include "../common/diagnostic.hpp"
#include "../ast/ast.hpp"
#include "pattern.hpp"

#include <cstddef>
#include <functional>
#include <optional>
#include <type_traits>
#include <vector>
#include <ostream>

#define ENSURE_NOT_EOF_BOOL()                                                 \
do                                                                            \
if (this->tokens.is_eof()) {                                                  \
	Diagnostic(DiagnosticKind::UNEXPECTED_EOF, this->tokens.prev())           \
		.emit(this->err_stream);                                              \
	return false;                                                             \
}                                                                             \
while (0)                                                                     

namespace scr {

class Parser {
private:
	// Arena allocator for nodes.
	BumpArena& arena;

	SourceStream<const std::span<Token>, Token, const Token&> tokens;

	std::ostream& err_stream;

	// Abstract Syntax Tree separated into each statements.
	std::vector<ASTNode> ast;

public:
	Parser(
		const std::span<Token> tokens,
		BumpArena& arena, std::ostream& err_stream) :
		tokens(tokens),
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
	bool validate_sprite_directive();
	
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

	// Parse `IDENTIFIER : "type"`, only work for variable declaration and
	// function arguments.
	template<typename T>
	requires (std::is_same_v<T, VarDeclarationStmt*> 
		|| std::is_same_v<T, FuncArgument*>)
	bool parse_type_annotation(T node) {
		// Ensure correct type annotation syntax.
		if (!Pattern<
				TokenKind::IDENTIFIER,
				TokenKind::COLON,
				TokenKind::IDENTIFIER>
					::match_peek(this->tokens, this->err_stream)) {
			return false;
		}
		
		node->name = 
			new_primary_node(ASTNodeKind::IDENTIFIER, this->tokens.advance());
		this->tokens.advance(); // Skip colon (':').
		node->type =
			new_primary_node(ASTNodeKind::TYPE, this->tokens.advance());

		return true;
	}

	// Usually parse arguments as `FuncArgument` but can also parse as expression.
	bool parse_func_args(std::vector<ASTNode>& buf, bool as_expr = false);

	inline bool expect(TokenKind kind) {
		if (match_token(kind)) {
			return true;
		} else {
			Diagnostic(resolve_diag_expect_kind(kind), this->tokens.prev())
				.emit(this->err_stream);
			return false;
		}
	}

	inline bool expect_peek(TokenKind kind) {
		if (this->tokens.peek().kind == kind) {
			return true;
		} else {
			Diagnostic(resolve_diag_expect_kind(kind), this->tokens.peek())
				.emit(this->err_stream);
			return false;
		}
	}

	inline bool match_token(TokenKind kind) {
		return this->tokens.match(
			[kind](auto token) -> bool { return token.kind == kind; });
	}

	inline ASTNode new_primary_node(ASTNodeKind kind, const Token& token) {
		auto node = this->arena.alloc<PrimaryExpr>();
		node->kind = kind;
		node->token = token;
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
