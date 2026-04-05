#pragma once

#include "ast.hpp"
#include "bytecode.hpp"
#include "const_pool.hpp"
#include "ast.hpp"

#include <ostream>
#include <vector>

namespace scr {

class CodeGenerator {
private:
	std::vector<Instruction> instructions;

	const std::vector<ASTNode>& ast;
	ConstPool const_pool = ConstPool();

public:
	CodeGenerator(const std::vector<ASTNode>& ast) :
		ast(ast)
	{ }

	bool generate();

	void output_inst(std::ostream& stream);

private:
	bool handle_node(const ASTNode& node);
};

} // namespace cr
