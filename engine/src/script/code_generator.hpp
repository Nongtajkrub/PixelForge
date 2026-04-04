#pragma once

#include "ast.hpp"
#include "bytecode.hpp"
#include "const_pool.hpp"

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

	bool handle_var_declaration(const ASTNode& ast_node);
	bool handle_literal(const ASTNode& ast_node);
	bool handle_identifier(const ASTNode& ast_node);

	Const const_from_literal(const ASTNode& ast_node);

	inline void generate_const(const Const& entry) {
		this->instructions.push_back(
			Instruction(
				OpCode::CONST, this->const_pool.intern_const(entry)));
	}
};

} // namespace cr
