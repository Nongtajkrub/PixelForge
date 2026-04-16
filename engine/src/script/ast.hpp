#pragma once

#include "../core/cplusplus/types.hpp"
#include "specs.h"
#include "symbol_table.hpp"
#include "const_pool.hpp"
#include "token.hpp"

#include <optional>
#include <ostream>
#include <vector>

namespace scr {

enum class ASTNodeKind : u8 {
	NOP,

	BLOCK,
	VAR_DECLARATION,
	FUNC_DECLARATION,
	FUNC_ARGUMENTS,
	ASSIGN,
	IF,
	FOR_LOOP,
	LOOP,

	BINARY,
	CALL,
	DOT,
	RANGE,

	DIRECTIVE,
	COMMAND,

	LITERAL,
	IDENTIFIER,
	TYPE,
	KEYWORD,
	BREAK,
	CONTINUE,
	RETURN,
};

// Forward declaration.
struct BlockStmt;

// Store info about where the node is stored in memory and how to interpret it.
// Since first member of every kind of expression is its kind.
struct ASTNode {
	// Hold a pointer to different nodes.
	const ASTNodeKind* adr;

	explicit ASTNode() = default;
	explicit ASTNode(const ASTNodeKind* adr) :
		adr(adr)
	{ }

	// Turn ASTNodeKind enum into string.
	const char* kind_as_str() const;

	std::vector<const BlockStmt*> get_child_blocks() const;

	void output(std::ostream& stream, const u32 level = 0) const;
};

struct AtomicNode {
	ASTNodeKind kind;
};

struct DirectiveStmt {
	ASTNodeKind kind;
	
	ASTNode directive;
	ASTNode identifier;
};

struct PrimaryExpr {
	ASTNodeKind kind;

	Token token;
}; 

struct LiteralExpr {
	ASTNodeKind kind;

	ConstIndex index;
};

struct IdentifierExpr {
	ASTNodeKind kind;

	// Store as ID so lookups are faster.
	IdentifierId id;
	IdenAttr* attr;
};

struct DotExpr {
	ASTNodeKind kind;

	ASTNode object;
	char property;
};

struct BinaryExpr {
	ASTNodeKind kind;

	ASTNode left;
	Token op;
	ASTNode right;
};

struct CallExpr {
	ASTNodeKind kind;

	ASTNode identifier;
	std::vector<ASTNode> args;
};

struct RangeExpr {
	ASTNodeKind kind;

	ASTNode begin;
	ASTNode end;
	std::optional<ASTNode> step;
};

struct ReturnStmt {
	ASTNodeKind kind;

	std::optional<ASTNode> expr;
};

struct ForLoopStmt {
	ASTNodeKind kind;

	std::optional<ASTNode> it;
	ASTNode range;

	ASTNode block; 
};

struct BlockStmt {
	ASTNodeKind kind;

	// separated into statements.
	std::vector<ASTNode> block;
};

struct VarDeclarationStmt {
	ASTNodeKind kind;

	ASTNode identifier;
	std::optional<ASTNode> init;
	ASTNode type;
};

struct FuncArgument {
	ASTNodeKind kind;

	ASTNode identifier;
	ASTNode type;
};

struct FuncDeclarationStmt {
	ASTNodeKind kind;
	
	ASTNode identifier;
	std::vector<ASTNode> args;
	ASTNode type;

	ASTNode body;
};

struct AssignStmt {
	ASTNodeKind kind;

	ASTNode identifier;
	ASTNode expr;
};

struct IfStmt {
	ASTNodeKind kind;

	ASTNode expr;

	ASTNode then_branch;
	std::optional<ASTNode> else_branch;
};

struct CommandStmt {
	ASTNodeKind kind;

	command_id_t id;
	ASTNode target;
	std::vector<ASTNode> args;
};

} // namespace scr
