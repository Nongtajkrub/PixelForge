#pragma once

#include "../global.hpp"
#include "token.hpp"
#include <optional>
#include <ostream>
#include <vector>

namespace scr {

enum class ASTNodeKind : u8 {
	NOP,

	VAR_DECLARATION,
	FUNC_DECLARATION,

	BINARY,
	CALL,

	LITERAL,
	IDENTIFIER,
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

struct VarDeclarationStmt {
	ASTNodeKind kind;

	ASTNode name;
	std::optional<ASTNode> init;
};

struct FuncDeclarationStmt {
	ASTNodeKind kind;

	ASTNode name;
	std::vector<ASTNode> args;

	// Function definition separated into statements.
	std::vector<ASTNode> body;
};

void ast_output(std::ostream& stream, ASTNode root, const u32 level = 0);

} // namespace scr
