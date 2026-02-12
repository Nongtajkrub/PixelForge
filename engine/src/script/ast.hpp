#pragma once

#include "../global.hpp"
#include "token.hpp"

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
	const ASTNodeKind kind = ASTNodeKind::LITERAL;

	Token token;
}; 

struct IdentifierExpr {
	const ASTNodeKind kind = ASTNodeKind::IDENTIFIER;

	Token token;
}; 

struct BinaryExpr {
	const ASTNodeKind kind = ASTNodeKind::BINARY;

	ASTNode* left;
	Token op;
	ASTNode* right;
};

struct DeclarationStmt {
	const ASTNodeKind kind = ASTNodeKind::DECLARATION;

	Token name;
	ASTNode* init;
};

} // namespace scr
