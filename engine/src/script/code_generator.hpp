#pragma once

#include "ast.hpp"
#include "symbol_table.hpp"
#include "vm_def.h"

#include <cassert>
#include <cstddef>
#include <ostream>
#include <queue>
#include <variant>
#include <vector>

namespace scr {

enum class CodeEntryKind : u8 {
	INSTRUCTION,
	LABEL,
};

enum class Label : u8 {
	HOLE,
	THEN_BRANCH,
	ELSE_BRANCH,
	IF_END,
};

struct CodeEntry {
	CodeEntryKind kind;
	std::variant<instruction_t, Label> data;

	explicit CodeEntry(instruction_t inst) :
		kind(CodeEntryKind::INSTRUCTION), data(inst)
	{ }
	explicit CodeEntry(Label label) :
		kind(CodeEntryKind::LABEL), data(label)
	{ }
};

class CodeGenerator {
private:
	std::vector<CodeEntry> code;

	// Store index of hole labels in the code avoiding o(n) lookup.
	std::queue<size_t> hole_indexes;

	const std::vector<ASTNode>& ast;

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
	void handle_if_stmt(const IfStmt* node);
	void handle_expr(const ASTNode& expr);
	void handle_binary_operator(TokenKind op);

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
		this->code.emplace_back(inst);
	}

	inline void push(Label label) {
		this->code.emplace_back(label);
		
		if (label == Label::HOLE) {
			this->hole_indexes.push(this->code.size() - 1);
		}
	}

	// Fill the first hole that appear in the code or push if no hole exist.
	inline void patch_next_hole(instruction_t inst) {
		if (hole_indexes.size() > 0) {
			this->code[hole_indexes.front()] = CodeEntry(inst);
			hole_indexes.pop();
		} else {
			push(inst);
		}
	}

	// Fill the first hole that appear in the code or push if no hole exist.
	inline void patch_next_hole(Label label) {
		if (hole_indexes.size() > 0) {
			this->code[hole_indexes.front()] = CodeEntry(label);
			hole_indexes.pop();
		} else {
			push(label);
		}
	}
};

} // namespace scr
