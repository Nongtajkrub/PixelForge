#pragma once

#include "../ast/ast.hpp"

#include <span>
#include <initializer_list>

namespace scr {

class Compiler {
private:
	std::vector<u8> bytes;

	const std::span<ASTNode> nodes;

public:
	Compiler(const std::span<ASTNode> nodes) :
		bytes({ }), nodes(nodes)
	{ }

	bool compile();

private:
	bool comp_var_declaration(ASTNode raw_node);
	bool comp_expr(ASTNode raw_node);

	inline void push_bytes(std::initializer_list<u8> bytes) {
		this->bytes.reserve(bytes.size());
		for (const auto byte : bytes) {
			this->bytes.push_back(byte);
		}
	}
};

} // namespace scr
