#pragma once

#include "ast.hpp"
#include "const_pool.hpp"
#include "ast.hpp"
#include "symbol_table.hpp"
#include "vm_def.h"

#include <cassert>
#include <ostream>
#include <vector>

namespace scr {

class CodeGenerator {
private:
	std::vector<instruction_t> code;

	const std::vector<ASTNode>& ast;
	ConstPool const_pool = ConstPool();

public:
	CodeGenerator(const std::vector<ASTNode>& ast) :
		ast(ast)
	{ }

	bool generate();

	void output_code(std::ostream& stream);

private:
	bool handle_node(const ASTNode& node);

	void handle_var_declaration(const VarDeclarationStmt* node);
	void handle_assign_stmt(const AssignStmt* node);
	void handle_expr(const ASTNode& expr);

	inline void generate_const(const LiteralExpr* node) {
		push(OP_CONST);
		push(node->index);
	}

	inline void generate_load(const IdentifierExpr* node) {
		assert(node->attr->kind == IdenKind::VAR);
		push(OP_LOAD);
		push(*(node->attr->slot));
	}

	inline void generate_store(const IdentifierExpr* node) {
		assert(node->attr->kind == IdenKind::VAR);
		push(OP_STORE);
		push(*(node->attr->slot));
	}

	inline void push(instruction_t inst) {
		this->code.push_back(inst);
	}
};

} // namespace scr
