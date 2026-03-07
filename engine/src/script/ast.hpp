#pragma once

#include "../global.hpp"
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
	IF,

	BINARY,
	CALL,

	LITERAL,
	IDENTIFIER,
	TYPE,
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

struct NopNode {
	ASTNodeKind kind;
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

struct IfStmt {
	ASTNodeKind kind;

	ASTNode expr;

	ASTNode then_branch;
	std::optional<ASTNode> else_branch;
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

	ASTNode body;
};

void ast_output(std::ostream& stream, ASTNode root, const u32 level = 0);

} // namespace scr
