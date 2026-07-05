#pragma once

#include "../core/cplusplus/utilities/ref_state_guard.hpp"
#include "../core/cplusplus/utilities/variant.hpp"
#include "fscript_symbol_table.hpp"
#include "fscript_ast.hpp"
#include "fscript_specs.h"
#include "vm/fscript_instruction.h"

#include <cassert>
#include <cstddef>
#include <ostream>
#include <vector>
#include <queue>

namespace scr {

enum class LabelKind : u8 {
	HOLE,
	THEN_BRANCH,
	ELSE_BRANCH,
	LOOP_BEGIN,
	LOOP_END,
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
	word_t index;
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
	IdInterner<IdentifierId, word_t> func_id_interner;

	// Update functions are automatically call every frame.
	std::vector<FuncEntry> updates;

	// Store index of hole labels in the code avoiding o(n) lookup.
	std::queue<size_t> hole_indexes;

	const std::vector<ASTNode>& ast;

public:
	CodeGenerator(const std::vector<ASTNode>& ast) :
		ast(ast),
		func_id_interner([this]() -> word_t { return this->func.size(); })
	{ 
		generate();
	}

	void output_code(std::ostream& stream);

	void serialize(std::vector<u8>& buf) const;

private:
	void handle_node(const ASTNode& node);

	void handle_var_declaration(const VarDeclarationStmt* node);
	void handle_func_declaration(const FuncDeclarationStmt* node);
	void handle_return_stmt(const ReturnStmt* node);
	void handle_assign_stmt(const AssignStmt* node);
	void handle_if_stmt(const IfStmt* node);
	void handle_for_stmt(const ForLoopStmt* node);
	void handle_loop_stmt(const LoopStmt* node);
	void handle_command(const CommandStmt* node);
	void handle_atomic_node(const AtomicNode* node);
	void handle_expr(const ASTNode& expr);
	void handle_binary_operator(TokenKind op);

	// Fill the first hole that appear in the code or push if no hole exist.
	void patch_next_hole(word_t inst);
	void patch_next_hole(Label label);

	// Get the offset of the first none reference label that appear from 
	// a specific point (Ignore none reference labels).
	size_t next_label_offset(
		const std::vector<CodeEntry>& src, size_t from, LabelKind label) const;
	size_t prev_label_offset(
		const std::vector<CodeEntry>& src, size_t from, LabelKind label) const;

	size_t serialize(std::vector<u8>& buf, const std::vector<CodeEntry>& src) const;

	static void output_code(
		std::ostream& stream, const std::vector<CodeEntry>& code);

	inline void generate() {
		generate(this->ast);
		push(OP_END);
	}

	inline void generate(const std::vector<ASTNode>& nodes) {
		for (const auto& node : nodes) {
			handle_node(node);
		}
	}

	inline void generate_load(const IdentifierExpr* node) {
		assert(node->attr->kind_is<VarAttr>());
		push(OP_LOAD);
		push(node->attr->get_data<VarAttr>().offset);
	}

	inline void generate_store(const IdentifierExpr* node) {
		assert(node->attr->kind_is<VarAttr>());
		push(OP_STORE);
		push(node->attr->get_data<VarAttr>().offset);
	}

	inline void generate_load_prop(const DotExpr* node) {
		handle_expr(node->object);
		push(OP_LOAD_PROP);
		push(node->offset);
	}

	inline void generate_store_prop(const DotExpr* node) {
		handle_expr(node->object);
		push(OP_STORE_PROP);
		push(node->offset);
	}

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
};

} // namespace scr
