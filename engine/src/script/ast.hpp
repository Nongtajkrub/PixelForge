#pragma once

#include "../global.hpp"
#include "token.hpp"
#include <optional>

namespace scr {

enum class ASTNodeKind : u8 {
	BINARY,
	STATEMENT,
	LITERAL,
	IDENTIFIER,
	DECLARATION,
};

// Store info about where the node is stored in memory and how to interpret it.
// Since first member of every kind of expression is its kind.
struct ASTNode {
	const ASTNodeKind* adr;

	ASTNode(const ASTNodeKind* adr) :
		adr(adr)
	{ }
};

struct LiteralExpr {
	ASTNodeKind kind;

	Token token;
}; 

struct IdentifierExpr {
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

} // namespace scr
