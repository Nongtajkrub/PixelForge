#include "code_generator.hpp"

#include "../core/cplusplus/io/byte_io.hpp"
#include "../core/cplusplus/macros.hpp"
#include "vm/instruction.h"
#include "symbol_table.hpp"
#include "token.hpp"
#include "ast.hpp"

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <ostream>
#include <utility>
#include <vector>

namespace scr {

std::vector<u8> CodeGenerator::serialize() {
	std::vector<u8> buff;
	buff.reserve((this->code.size() + this->func.size()) * sizeof(word_t));

	for (size_t i = 0; i < this->code.size(); i++) {
		const auto& entry = this->code[i];

		if (entry.data.is<instruction_t>()) {
			push_bytes(buff, entry.data.get<instruction_t>());
		} else if (entry.data.is<Label>()) {
			const auto label = entry.data.get<Label>();

			if (label.kind == LabelKind::HOLE || !label.is_ref) {
				continue;
			}

			switch (label.kind) {
			// Scan forward.
			case LabelKind::ELSE_BRANCH:
			case LabelKind::THEN_BRANCH:
			case LabelKind::IF_END:
			case LabelKind::RETURN:
			case LabelKind::RETURN_ADDR:
				push_bytes<word_t>(
					buff, buff.size() + next_label_offset(i + 1, label.kind));
				break;
			// Scan backward.
			case LabelKind::LOOP_BEGIN:
				push_bytes<word_t>(
					buff, buff.size() - prev_label_offset(i - 1, label.kind));
				break;
			case LabelKind::HOLE:
				std::unreachable();
			}
		}
	}

	return buff;
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
	case ASTNodeKind::CALL:
		handle_expr(node);
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
	
	generate_store(
		reinterpret_cast<const IdentifierExpr*>(node->identifier.adr));
}

void CodeGenerator::handle_func_declaration(const FuncDeclarationStmt* node) {
	auto& entry = this->func.emplace_back();

	const auto identifier =
		reinterpret_cast<const IdentifierExpr*>(node->identifier.adr);
	entry.id = identifier->id;

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

void CodeGenerator::handle_command(const CommandStmt* node) {
	for (const auto& arg : node->args) {
		handle_expr(arg);
	}

	handle_expr(node->target);

	push(OP_COMMAND);
	push(static_cast<word_t>(node->id));
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
		LOG_ERR("Unimplemented operator for code generator.");
		exit(1);
	}
}

void CodeGenerator::handle_expr(const ASTNode& expr) {
	switch (*expr.adr) {
	case ASTNodeKind::LITERAL:
		generate_const(reinterpret_cast<const LiteralExpr*>(expr.adr));
		break;
	case ASTNodeKind::IDENTIFIER: {
		generate_load(reinterpret_cast<const IdentifierExpr*>(expr.adr));
		break;
	}
	case ASTNodeKind::DOT: {
		const auto node = reinterpret_cast<const DotExpr*>(expr.adr);

		generate_load(reinterpret_cast<const IdentifierExpr*>(node->object.adr));

		push(OP_LOAD_PROP);
		push(static_cast<u8>(node->property));

		break;
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

		break;
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

		push(OP_CALL);
		push(reinterpret_cast<const IdentifierExpr*>(node->identifier.adr)->id);
		push(Label(LabelKind::RETURN_ADDR, false));

		break;
	}
	case ASTNodeKind::RANGE:
		TODO();
		break;
	default:
		LOG_ERR("Node is not an expression.");
		exit(1);
	}
}

void CodeGenerator::generate_load(const IdentifierExpr* node) {
	assert(node->attr->kind_is<VarAttr>() || node->attr->kind_is<ArgAttr>());

	if (node->attr->kind_is<VarAttr>()) {
		push(OP_LOAD);
		push(node->attr->get_data<VarAttr>().slot);
	} else if (node->attr->kind_is<ArgAttr>()) {
		push(OP_LOAD_STACK);
		push(node->attr->get_data<ArgAttr>().index);
	} else {
		LOG_ERR("Unimplemented identifier attribute loading");
		exit(1);
	}
}

void CodeGenerator::generate_store(const IdentifierExpr* node) {
	assert(node->attr->kind_is<VarAttr>() || node->attr->kind_is<ArgAttr>());

	if (node->attr->kind_is<VarAttr>()) {
		push(OP_STORE);
		push(node->attr->get_data<VarAttr>().slot);
	} else if (node->attr->kind_is<ArgAttr>()) {
		push(OP_STORE_STACK);
		push(node->attr->get_data<ArgAttr>().index);
	} else {
		LOG_ERR("Unimplemented identifier attribute loading");
		exit(1);
	}
}

size_t CodeGenerator::next_label_offset(size_t from, LabelKind label) {
	assert(from < this->code.size());

	size_t offset = 0;
	for (size_t i = from; i < this->code.size(); i++) {
		const auto& entry = this->code[i];

		if (entry.data.is<Label>()) {
			const auto entry_label = entry.data.get<Label>();

			if (entry_label.kind == label && !entry_label.is_ref) {
				return offset + 1;
			}
		} else {
			offset++;
		}
	}

	LOG_ERR("Code is not correctly generated");
	exit(1);
}

size_t CodeGenerator::prev_label_offset(size_t from, LabelKind label) {
	assert(from < this->code.size());

	size_t offset = 0;
	for (size_t i = from; i-- > 0;) {
		const auto& entry = this->code[i];

		if (entry.data.is<Label>()) {
			const auto entry_label = entry.data.get<Label>();

			if (entry_label.kind == label && !entry_label.is_ref) {
				return offset + 1;
			}
		} else {
			offset++;
		}
	}

	LOG_ERR("Code is not correctly generated");
	exit(1);
}

static const char* label_to_str(LabelKind label) {
	switch (label) {
	case LabelKind::HOLE: return "HOLE"; 
	case LabelKind::THEN_BRANCH: return "THEN_BRANCH";
	case LabelKind::ELSE_BRANCH: return "ELSE_BRANCH";
	case LabelKind::IF_END: return "IF_END";
	case LabelKind::LOOP_BEGIN: return "LOOP_BEGIN";
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
		stream << "\tFUNC_ID: " << entry.id << '\n';
		output_code(stream, entry.body);
	}
}

void CodeGenerator::output_serialize(
	std::ostream& stream, const std::vector<instruction_t>& buffer) {
	for (size_t i = 0; i < buffer.size(); i++) {
		const auto op = static_cast<opcode_t>(buffer[i]);

		stream << op_to_str(op); 

		if (op_have_operand(op)) {
			i++;
			assert(i < buffer.size());
			stream << '\n' << buffer[i];
		}

		stream << '\n';
	}
}

} // namespace scr
