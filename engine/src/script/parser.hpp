#pragma once

#include "../util/bump_arena.hpp"
#include "source_stream.hpp"
#include "token.hpp"
#include "ast.hpp"

#include <cstddef>
#include <vector>

namespace scr {

class Parser {
private:
	// Arena allocator for nodes.
	BumpArena& arena;

	SourceStream<const std::vector<Token>&, Token, const Token&> tokens;

	// Abstract Syntax Tree separated into each line.
	std::vector<ASTNode*> ast;

public:
	Parser(const std::vector<Token>& tokens, BumpArena& arena) :
		tokens(tokens),
		arena(arena)
	{ }

	void parse();
};

} // namespace scr
