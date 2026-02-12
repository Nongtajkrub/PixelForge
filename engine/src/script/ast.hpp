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

// Store info about how to interpretate expression memory.
struct ASTNode {
	ASTNodeKind kind;
};

struct LiteralExpr {
	ASTNodeKind kind = ASTNodeKind::LITERAL;

	Token token;
}; 

struct IdentifierExpr {
	ASTNodeKind kind = ASTNodeKind::IDENTIFIER;

	Token token;
}; 

struct BinaryExpr {
	ASTNodeKind kind = ASTNodeKind::BINARY;

	ASTNode* left;
	Token op;
	ASTNode* right;
};

struct DeclarationStmt {
	ASTNodeKind kind = ASTNodeKind::DECLARATION;

	Token name;
	std::optional<ASTNode*> init;
};

} // namespace scr
