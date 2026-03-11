#pragma once

#include "../../global.hpp"
#include "../../core/arena/bump_arena.hpp"
#include "../common/token.hpp"

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

	BINARY,
	CALL,

	DIRECTIVE,
	COMMAND,

	LITERAL,
	IDENTIFIER,
	TYPE,
	KEYWORD,
};

// Store info about where the node is stored in memory and how to interpret it.
// Since first member of every kind of expression is its kind.
struct ASTNode {
	// Hold a pointer to different nodes.
	const ASTNodeKind* adr;

	ASTNode(const ASTNodeKind* adr) :
		adr(adr)
	{ }

	// Turn ASTNodeKind enum into string.
	const char* kind_as_str() const;
};

// Buffer allowing external function without onwership of the arena containing
// nodes to operate with a node buffer like "BlockStmt".
struct ASTNodeBuffer {
	BumpArena& arena;
	std::vector<ASTNode>& buf;

	ASTNodeBuffer(BumpArena& arena, std::vector<ASTNode>& buf) :
		arena(arena), buf(buf)
	{ }
}; 

struct NopNode {
	ASTNodeKind kind;
};

struct DirectiveStmt {
	ASTNodeKind kind;
	
	ASTNode directive;
	std::optional<ASTNode> expr;
};

struct PrimaryExpr {
	ASTNodeKind kind;

	Token token;
}; 

struct BinaryExpr {
	ASTNodeKind kind;

	ASTNode left;
	Token op;
	ASTNode right;
};

struct CallExpr {
	ASTNodeKind kind;

	ASTNode name;
	std::vector<ASTNode> args;
};

struct BlockStmt {
	ASTNodeKind kind;

	// separated into statements.
	std::vector<ASTNode> block;
};

struct VarDeclarationStmt {
	ASTNodeKind kind;

	ASTNode name;
	ASTNode type;
	std::optional<ASTNode> init;
};

struct FuncArgument {
	ASTNodeKind kind;

	ASTNode name;
	ASTNode type;
};

struct FuncDeclarationStmt {
	ASTNodeKind kind;

	ASTNode name;
	std::vector<ASTNode> args;
	ASTNode type;

	ASTNode body;
};

struct AssignStmt {
	ASTNodeKind kind;

	ASTNode iden;
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

	ASTNode command;
	ASTNode target;
	std::vector<ASTNode> operands;
};

void ast_output(std::ostream& stream, ASTNode root, const u32 level = 0);

} // namespace scr
