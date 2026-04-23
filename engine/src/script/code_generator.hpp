#pragma once

#include "../core/cplusplus/utilities/ref_state_guard.hpp"
#include "../core/cplusplus/utilities/variant.hpp"
#include "vm/instruction.h"
#include "symbol_table.hpp"
#include "ast.hpp"

#include <cassert>
#include <cstddef>
#include <ostream>
#include <queue>
#include <vector>

namespace scr {

enum class LabelKind : u8 {
	HOLE,
	THEN_BRANCH,
	ELSE_BRANCH,
	LOOP_BEGIN,
	IF_END,
	RETURN_ADDR,
	RETURN,
};

struct Label {
	LabelKind kind;

	// Label will be a reference when it is use as operands.
	bool is_ref;

	Label(LabelKind kind, bool is_ref) :
		kind(kind), is_ref(is_ref)
	{ }
};

struct CodeEntry {
	Variant<instruction_t, Label> data;

	explicit CodeEntry(instruction_t inst) :
		data(inst)
	{ }
	explicit CodeEntry(Label label) :
		data(label)
	{ }
};

struct FuncEntry {
	IdentifierId id;
	std::vector<CodeEntry> body;
};

class CodeGenerator {
private:
	// Which buffer to push code to (Default is the main code buffer).
	// Store as pointer to avoid copy when using state guard.
	std::vector<CodeEntry>* push_buffer = &this->code;

	// Main code buffer.
	std::vector<CodeEntry> code;
	// Function code buffer containing function definitions and implementations.
	std::vector<FuncEntry> func;

	// Store index of hole labels in the code avoiding o(n) lookup.
	std::queue<size_t> hole_indexes;

	const std::vector<ASTNode>& ast;

public:
	CodeGenerator(const std::vector<ASTNode>& ast) :
		ast(ast)
	{ }

	inline void generate() {
		generate(this->ast);
		push(OP_END);
	}

	void output_code(std::ostream& stream);
	void output_serialize(
		std::ostream& stream, const std::vector<word_t>& buffer);

	std::vector<u8> serialize();

private:
	static void output_code(
		std::ostream& stream, const std::vector<CodeEntry>& code);

	void handle_node(const ASTNode& node);

	void handle_var_declaration(const VarDeclarationStmt* node);
	void handle_func_declaration(const FuncDeclarationStmt* node);
	void handle_return_stmt(const ReturnStmt* node);
	void handle_assign_stmt(const AssignStmt* node);
	void handle_if_stmt(const IfStmt* node);
	void handle_for_stmt(const ForLoopStmt* node);
	void handle_command(const CommandStmt* node);
	void handle_expr(const ASTNode& expr);
	void handle_binary_operator(TokenKind op);

	void generate_load(const IdentifierExpr* node);
	void generate_store(const IdentifierExpr* node);

	inline void generate(const std::vector<ASTNode>& nodes) {
		for (const auto& node : nodes) {
			handle_node(node);
		}
	}

	// Get the offset of the first none reference label that appear from 
	// a specific point (Ignore none reference labels).
	size_t next_label_offset(size_t from, LabelKind label);
	size_t prev_label_offset(size_t from, LabelKind label);

	// Allow for switching push buffer with a state guard.
	[[nodiscard]]
	inline RefStateGuard<std::vector<CodeEntry>> switch_push_buffer(
		std::vector<CodeEntry>* buffer) {
		return RefStateGuard<std::vector<CodeEntry>>(this->push_buffer, buffer);
	}

	inline void generate_const(const LiteralExpr* node) {
		push(OP_CONST);
		push(node->index);
	}

	inline void push(word_t data) {
		this->push_buffer->emplace_back(data);
	}

	inline void push(Label label) {
		this->push_buffer->emplace_back(label);
		
		if (label.kind == LabelKind::HOLE) {
			this->hole_indexes.push(this->code.size() - 1);
		}
	}

	// Fill the first hole that appear in the code or push if no hole exist.
	inline void patch_next_hole(word_t inst) {
		if (hole_indexes.size() > 0) {
			(*this->push_buffer)[hole_indexes.front()] = CodeEntry(inst);
			hole_indexes.pop();
		} else {
			push(inst);
		}
	}

	// Fill the first hole that appear in the code or push if no hole exist.
	inline void patch_next_hole(Label label) {
		if (hole_indexes.size() > 0) {
			(*this->push_buffer)[hole_indexes.front()] = CodeEntry(label);
			hole_indexes.pop();
		} else {
			push(label);
		}
	}
};

} // namespace scr
