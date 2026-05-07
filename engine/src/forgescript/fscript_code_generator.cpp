#include "fscript_code_generator.hpp"

#include "../core/cplusplus/io/byte_io.hpp"
#include "../core/cplusplus/macros.hpp"
#include "fscript_symbol_table.hpp"
#include "fscript_token.hpp"
#include "fscript_ast.hpp"
#include "fscript_specs.h"
#include "vm/fscript_instruction.h"

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <ostream>
#include <vector>

namespace scr {

void CodeGenerator::serialize(std::vector<u8>& buf) const {
	push_bytes<word_t>(buf, this->code.size() * WORD_SIZE);
	serialize(buf, this->code);

	std::vector<u8> func_buf;
	for (const auto& entry : this->func) {
		push_bytes<word_t>(func_buf, entry.body.size() * WORD_SIZE);
		serialize(func_buf, entry.body);
	}

	// Concat two buffer together with size information.
	buf.reserve(WORD_SIZE + func_buf.size());
	push_bytes<word_t>(buf, func_buf.size());
	buf.insert(buf.end(), func_buf.begin(), func_buf.end());
}

void CodeGenerator::handle_node(const ASTNode& node) {
	switch (*node.adr) {
	case ASTNodeKind::VAR_DECLARATION:
		handle_var_declaration(
			reinterpret_cast<const VarDeclarationStmt*>(node.adr));
		break;
	case ASTNodeKind::FUNC_DECLARATION:
		handle_func_declaration(
			reinterpret_cast<const FuncDeclarationStmt*>(node.adr));
		break;
	case ASTNodeKind::ASSIGN:
		handle_assign_stmt(reinterpret_cast<const AssignStmt*>(node.adr));
		break;
	case ASTNodeKind::IF:
		handle_if_stmt(reinterpret_cast<const IfStmt*>(node.adr));
		break;
	case ASTNodeKind::FOR_LOOP:
		handle_for_stmt(reinterpret_cast<const ForLoopStmt*>(node.adr));
		break;
	case ASTNodeKind::LOOP:
		handle_loop_stmt(reinterpret_cast<const LoopStmt*>(node.adr));
		break;
	case ASTNodeKind::CALL:
		handle_expr(node);
		break;
	case ASTNodeKind::BREAK:
	case ASTNodeKind::CONTINUE:
		handle_atomic_node(reinterpret_cast<const AtomicNode*>(node.adr));
		break;
	case ASTNodeKind::RETURN:
		handle_return_stmt(reinterpret_cast<const ReturnStmt*>(node.adr));
		break;
	case ASTNodeKind::COMMAND:
		handle_command(reinterpret_cast<const CommandStmt*>(node.adr));
		break;
	case ASTNodeKind::NOP:
		push(OP_NOP);
		break;
	default:
		break;
	}
}

void CodeGenerator::handle_var_declaration(const VarDeclarationStmt* node) {
	if (node->init) {
		handle_expr(*(node->init));
	}

	generate_store(
		reinterpret_cast<const IdentifierExpr*>(node->identifier.adr));
}

void CodeGenerator::handle_assign_stmt(const AssignStmt* node) {
	handle_expr(node->expr);

	switch (*node->var.adr) {
	case ASTNodeKind::IDENTIFIER:
		generate_store(reinterpret_cast<const  IdentifierExpr*>(node->var.adr));
		break;
	case ASTNodeKind::DOT:
		generate_store_prop(reinterpret_cast<const DotExpr*>(node->var.adr));
		break;
	default:
		BUG("Unhandle node type for assignment");
		exit(1);
	}
}

void CodeGenerator::handle_func_declaration(const FuncDeclarationStmt* node) {
	const auto identifier =
		reinterpret_cast<const IdentifierExpr*>(node->identifier.adr);

	// Update function ID to index map (Do before pushing new entry).
	const auto [index, _] = this->func_id_interner.intern(identifier->id);

	auto& entry = this->func.emplace_back();
	entry.index = index;

	auto guard = switch_push_buffer(&entry.body);

	generate(reinterpret_cast<const BlockStmt*>(node->body.adr)->block);

	push(Label(LabelKind::RETURN, false));
	const size_t arg_n = identifier->attr->get_data<FuncAttr>().arg_n();
	for (size_t i = 0; i < arg_n; i++){
		push(OP_POP);
	}
	push(OP_RETURN);
}

void CodeGenerator::handle_return_stmt(const ReturnStmt* node) {
	if (node->expr) {
		handle_expr(*node->expr);
	}

	push(OP_JMP);
	push(Label(LabelKind::RETURN, true));
}

void CodeGenerator::handle_if_stmt(const IfStmt* node) {
	handle_expr(node->expr);

	push(OP_JMP_FALSE);
	push(Label(LabelKind::ELSE_BRANCH, true));

	// Handle then branch.
	push(Label(LabelKind::THEN_BRANCH, false));
	generate(reinterpret_cast<const BlockStmt*>(node->then_branch.adr)->block);

	// Push jump instruction to avoid executing else branch after then branch.
	push(OP_JMP);
	push(Label(LabelKind::IF_END, true));

	push(Label(LabelKind::ELSE_BRANCH, false));

	// Handle else branch if exist.
	if (node->else_branch) {
		generate(
			reinterpret_cast<const BlockStmt*>((*node->else_branch).adr)->block);
	} 

	push(Label(LabelKind::IF_END, false));
}

void CodeGenerator::handle_for_stmt(const ForLoopStmt* node) {
	const auto range_node = reinterpret_cast<const RangeExpr*>(node->range.adr);
	const auto it = reinterpret_cast<const IdentifierExpr*>(node->it.adr);

	// Store begin in iterator.
	handle_expr(range_node->begin);
	generate_store(it);

	// Generate code for loop block.
	push(Label(LabelKind::LOOP_BEGIN, false));
	generate(reinterpret_cast<const BlockStmt*>(node->block.adr)->block);

	// Increment iterator.
	generate_load(it);
	if (!range_node->step) {
		push(OP_CONST_DIRECT);
		push(1);
	} else {
		handle_expr(*range_node->step);
	}
	push(OP_ADD);
	generate_store(it);

	// Range checking.
	handle_expr(range_node->end);
	push(OP_COMPARE_LESS);
	push(OP_JMP);
	push(Label(LabelKind::LOOP_BEGIN, true));
}

void CodeGenerator::handle_loop_stmt(const LoopStmt* node) {
	if (node->is_update) {
		auto& entry = this->updates.emplace_back();
		entry.index = this->updates.size() - 1;

		auto guard = switch_push_buffer(&entry.body);

		generate(reinterpret_cast<const BlockStmt*>(node->block.adr)->block);
	} else {
		push(Label(LabelKind::LOOP_BEGIN, false));
		generate(reinterpret_cast<const BlockStmt*>(node->block.adr)->block);
		push(Label(LabelKind::LOOP_END, false));
	}
}

void CodeGenerator::handle_command(const CommandStmt* node) {
	for (const auto& arg : node->args) {
		handle_expr(arg);
	}

	push(OP_COMMAND);
	push(static_cast<word_t>(node->id));
}

void CodeGenerator::handle_atomic_node(const AtomicNode* node) {
	switch (node->kind) {
	case ASTNodeKind::BREAK:
		push(OP_JMP);
		push(Label(LabelKind::LOOP_END, true));
		break;
	case ASTNodeKind::CONTINUE:
		push(OP_JMP);
		push(Label(LabelKind::LOOP_BEGIN, true));
		break;
	default:
		BUG("Unhandled AtomicNode.");
		exit(1);
	}
}

void CodeGenerator::handle_binary_operator(TokenKind op) {
	switch (op) {
	case TokenKind::PLUS: 
		push(OP_ADD);
		break;
	case TokenKind::MINUS: 
		push(OP_MINUS);
		break;
	case TokenKind::STAR: 
		push(OP_MULTIPLY);
		break;
	case TokenKind::SLASH: 
		push(OP_DIVIDE);
		break;
	case TokenKind::DOUBLE_EQUAL: 
		push(OP_COMPARE_EQUAL);
		break;
	case TokenKind::BANG_EQUAL: 
		push(OP_COMPARE_NOT_EQUAL);
		break;
	case TokenKind::GREATER: 
		push(OP_COMPARE_GEATER);
		break;
	case TokenKind::LESS: 
		push(OP_COMPARE_LESS);
		break;
	case TokenKind::GREATER_EQUAL: 
		push(OP_COMPARE_GEATER_EQUAL);
		break;
	case TokenKind::LESS_EQUAL: 
		push(OP_COMPARE_LESS_EQUAL);
		break;
	case TokenKind::AND: 
		// Fill in space reserved by handle_expr for short circuiting.
		patch_next_hole(OP_JMP_FALSE);
		patch_next_hole(Label(LabelKind::ELSE_BRANCH, true));
		break;
	case TokenKind::OR:
		// Fill in space reserved by handle_expr for short circuiting.
		patch_next_hole(OP_JMP_TRUE);
		patch_next_hole(Label(LabelKind::THEN_BRANCH, true));
		break;
	default:
		BUG("Unimplemented operator for code generator.");
		exit(1);
	}
}

void CodeGenerator::handle_expr(const ASTNode& expr) {
	switch (*expr.adr) {
	case ASTNodeKind::LITERAL:
		generate_const(reinterpret_cast<const LiteralExpr*>(expr.adr));
		return;
	case ASTNodeKind::IDENTIFIER: {
		generate_load(reinterpret_cast<const IdentifierExpr*>(expr.adr));
		return;
	}
	case ASTNodeKind::COMMAND: {
		handle_command(reinterpret_cast<const CommandStmt*>(expr.adr));
		return;
	}
	case ASTNodeKind::DOT: {
		const auto node = reinterpret_cast<const DotExpr*>(expr.adr);

		handle_expr(node->object);
		push(OP_LOAD_PROP);
		push(node->offset);

		return;
	}
	case ASTNodeKind::BINARY: {
		const auto node = reinterpret_cast<const BinaryExpr*>(expr.adr);

		handle_expr(node->left);

		// Reserve space for short circuiting if logical operator are use.
		if (token_is_logical_operator(node->op.kind)) {
			push(Label(LabelKind::HOLE, false));
			push(Label(LabelKind::HOLE, false));
		}

		handle_expr(node->right);
		handle_binary_operator(node->op.kind);

		return;
	}
	case ASTNodeKind::CALL: {
		const auto node = reinterpret_cast<const CallExpr*>(expr.adr);

		// Pusing return address.
		push(Label(LabelKind::RETURN_ADDR, true));
		push(OP_PUSH);

		for (const auto arg : node->args) {
			handle_expr(arg);
			push(OP_PUSH);
		}

		const auto [index, inserted] =
			this->func_id_interner.intern(
				reinterpret_cast<const IdentifierExpr*>(node->identifier.adr)->id);

		if (inserted) {
			BUG("Function should already have their ID map before being call.");
			exit(1);
		}

		push(OP_CALL);
		push(index);
		push(Label(LabelKind::RETURN_ADDR, false));

		return;
	}
	case ASTNodeKind::CONSTRUCTOR: {
		const auto node = reinterpret_cast<const ConstructorExpr*>(expr.adr);

		push(OP_CONSTRUCT);

		for (const auto& arg : node->args) {
			handle_expr(arg);
			push(OP_CONSTRUCT_REGISTER);
		}
	}
	case ASTNodeKind::RANGE:
		TODO();
		return;
	default:
		break;
	}

	BUG("Node is not an expression.");
	exit(1);
}

void CodeGenerator::patch_next_hole(word_t inst) {
	if (hole_indexes.size() > 0) {
		(*this->push_buffer)[hole_indexes.front()] = CodeEntry(inst);
		hole_indexes.pop();
	} else {
		push(inst);
	}
}

// Fill the first hole that appear in the code or push if no hole exist.
void CodeGenerator::patch_next_hole(Label label) {
	if (hole_indexes.size() > 0) {
		(*this->push_buffer)[hole_indexes.front()] = CodeEntry(label);
		hole_indexes.pop();
	} else {
		push(label);
	}
}

size_t CodeGenerator::next_label_offset(
	const std::vector<CodeEntry>& src, size_t from, LabelKind label) const {
	assert(from < src.size());

	size_t offset = 0;
	for (size_t i = from; i < src.size(); i++) {
		const auto& entry = src[i];

		if (entry.data.is<Label>()) {
			const auto entry_label = entry.data.get<Label>();

			if (entry_label.kind == label && !entry_label.is_ref) {
				return offset + 1;
			}
		} else {
			offset++;
		}
	}

	BUG("Code is not correctly generated");
	exit(1);
}

size_t CodeGenerator::prev_label_offset(
	const std::vector<CodeEntry>& src, size_t from, LabelKind label) const {
	assert(from < src.size());

	size_t offset = 0;
	for (size_t i = from; i-- > 0;) {
		const auto& entry = src[i];

		if (entry.data.is<Label>()) {
			const auto entry_label = entry.data.get<Label>();

			if (entry_label.kind == label && !entry_label.is_ref) {
				return offset + 1;
			}
		} else {
			offset++;
		}
	}

	BUG("Code is not correctly generated");
	exit(1);
}

void CodeGenerator::serialize(
	std::vector<u8>& buf, const std::vector<CodeEntry>& src) const {
	buf.reserve(src.size() * WORD_SIZE);

	// Serialize main instructions.
	for (size_t i = 0; i < src.size(); i++) {
		const auto& entry = src[i];

		if (entry.data.is<instruction_t>()) {
			push_bytes(buf, entry.data.get<instruction_t>());
		} else if (entry.data.is<Label>()) {
			// Resolve labels.
			const auto label = entry.data.get<Label>();

			if (label.kind == LabelKind::HOLE || !label.is_ref) continue;

			switch (label.kind) {
			// Scan forward.
			case LabelKind::ELSE_BRANCH:
			case LabelKind::THEN_BRANCH:
			case LabelKind::IF_END:
			case LabelKind::RETURN:
			case LabelKind::RETURN_ADDR:
			case LabelKind::LOOP_END:
				push_bytes<word_t>(
					buf, buf.size() + next_label_offset(src, i + 1, label.kind));
				break;
			// Scan backward.
			case LabelKind::LOOP_BEGIN:
				push_bytes<word_t>(
					buf, buf.size() - prev_label_offset(src, i - 1, label.kind));
				break;
			case LabelKind::HOLE:
				BUG("LabelKind::HOLE should not be reach.");
				exit(1);
			}
		}
	}
}

static const char* label_to_str(LabelKind label) {
	switch (label) {
	case LabelKind::HOLE: return "HOLE"; 
	case LabelKind::THEN_BRANCH: return "THEN_BRANCH";
	case LabelKind::ELSE_BRANCH: return "ELSE_BRANCH";
	case LabelKind::LOOP_BEGIN: return "LOOP_BEGIN";
	case LabelKind::LOOP_END: return "LOOP_END";
	case LabelKind::IF_END: return "IF_END";
	case LabelKind::RETURN_ADDR: return "RETURN_ADDR";
	case LabelKind::RETURN: return "RETURN";
	}
}

void CodeGenerator::output_code(
	std::ostream& stream, const std::vector<CodeEntry>& code) {
	for (size_t i = 0; i < code.size(); i++) {
		const auto& entry = code[i];

		if (entry.data.is<instruction_t>()) {
			const auto op =
				static_cast<opcode_t>(entry.data.get<instruction_t>());

			stream << op_to_str(op); 

			if (op_have_operand(op)) {
				assert(i + 1 < code.size());
				const auto& operand_entry = code[++i];

				stream << ' ';
				if (operand_entry.data.is<instruction_t>()) {
					stream << operand_entry.data.get<instruction_t>();
				} else if (operand_entry.data.is<Label>()) {
					stream << label_to_str(operand_entry.data.get<Label>().kind);
				}
			}
		} else if (entry.data.is<Label>()) {
			stream << '\t' << label_to_str(entry.data.get<Label>().kind);
		}

		stream << '\n';
	}
}

void CodeGenerator::output_code(std::ostream& stream) {
	output_code(stream, this->code);

	for (const auto& entry : this->func) {
		stream << "\tFUNC_INDEX: " << entry.index << '\n';
		output_code(stream, entry.body);
	}

	for (const auto& entry : this->updates) {
		stream << "\tFUNC_INDEX: " << entry.index << '\n';
		output_code(stream, entry.body);
	}
}

} // namespace scr
