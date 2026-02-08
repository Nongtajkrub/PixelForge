#pragma once

#include "token.hpp"

#include <memory>
#include <optional>
#include <vector>

namespace scr {

struct ParseNode {
	// nullopt if node is root.
	std::optional<Token> token;

	ParseNode* parent = nullptr;
	std::vector<std::unique_ptr<ParseNode>> children;

	ParseNode() :
		token(std::nullopt)
	{ }
	ParseNode(Token token) :
		token(std::move(token))
	{ }
};

class ParseTree {
private:
	std::vector<ParseNode> tree;

public:
	inline void new_line() {
		this->tree.push_back(ParseNode());
	}
};

} // namespace scr
