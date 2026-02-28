#pragma once

#include "../global.hpp"
#include "token.hpp"
#include <optional>
#include <ostream>

namespace scr {

enum class ASTNodeKind : u8 {
	DECLARATION,

	BINARY,

	LITERAL,
	IDENTIFIER,
};

// Store info about where the node is stored in memory and how to interpret it.
// Since first member of every kind of expression is its kind.
struct ASTNode {
	const ASTNodeKind* adr;

	ASTNode(const ASTNodeKind* adr) :
		adr(adr)
	{ }

	const char* kind_as_str() const;
};

struct TerminateNode {
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

struct DeclarationStmt {
	ASTNodeKind kind;

	Token name;
	std::optional<ASTNode> init;
};

void ast_output(std::ostream& stream, ASTNode root, const u32 level = 0);

} // namespace scr
