#include "code_generator.hpp"

#include "../core/cplusplus/macros.hpp"
#include "vm/instruction.h"
#include "symbol_table.hpp"
#include "token.hpp"
#include "ast.hpp"

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <vector>

namespace scr {

static const char* label_to_str(Label label) {
	switch (label) {
	case Label::HOLE: return "HOLE"; 
	case Label::THEN_BRANCH: return "THEN_BRANCH";
	case Label::ELSE_BRANCH: return "ELSE_BRANCH";
	case Label::IF_END: return "IF_END";
	}
}

void CodeGenerator::output_code(
	std::ostream& stream, const std::vector<CodeEntry>& code) {
	for (size_t i = 0; i < code.size(); i++) {
		const auto& entry = code[i];

		if (entry.kind == CodeEntryKind::INSTRUCTION) {
			const auto op = static_cast<opcode_t>(entry.get_inst());

			stream << op_to_str(op); 

			if (op_have_operand(op)) {
				assert(i + 1 < code.size());
				const auto& operand_entry = code[++i];

				stream << ": ";
				if (operand_entry.kind == CodeEntryKind::INSTRUCTION) {
					stream << operand_entry.get_inst();
				} else {
					stream << label_to_str(operand_entry.get_label());
				}
			}
		} else if (entry.kind == CodeEntryKind::LABEL) {
			stream << '\t' << label_to_str(std::get<Label>(entry.data));
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
	case ASTNodeKind::CALL:
		handle_expr(node);
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
	const size_t arg_n = identifier->attr->data.get<FuncAttr>().args_types.size();
	for (size_t i = 0; i < arg_n; i++){
		push(OP_POP);
	}
}

void CodeGenerator::handle_if_stmt(const IfStmt* node) {
	handle_expr(node->expr);

	push(OP_JMP_FALSE);
	push(Label::ELSE_BRANCH);

	// Handle then branch.
	push(Label::THEN_BRANCH);
	generate(reinterpret_cast<const BlockStmt*>(node->then_branch.adr)->block);

	// Push jump instruction to avoid executing else branch after then branch.
	push(OP_JMP);
	push(Label::IF_END);

	push(Label::ELSE_BRANCH);

	// Handle else branch if exist.
	if (node->else_branch) {
		generate(
			reinterpret_cast<const BlockStmt*>((*node->else_branch).adr)->block);
	} 

	push(Label::IF_END);
}

void CodeGenerator::handle_command(const CommandStmt* node) {
	for (const auto& arg : node->args) {
		handle_expr(arg);
	}

	handle_expr(node->target);

	push(OP_COMMAND);
	push(static_cast<instruction_t>(node->id));
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
		patch_next_hole(Label::ELSE_BRANCH);
		break;
	case TokenKind::OR:
		// Fill in space reserved by handle_expr for short circuiting.
		patch_next_hole(OP_JMP_TRUE);
		patch_next_hole(Label::THEN_BRANCH);
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
			push(Label::HOLE);
			push(Label::HOLE);
		}

		handle_expr(node->right);
		handle_binary_operator(node->op.kind);

		break;
	}
	case ASTNodeKind::CALL: {
		const auto node = reinterpret_cast<const CallExpr*>(expr.adr);

		for (const auto arg : node->args) {
			handle_expr(arg);
			push(OP_PUSH);
		}

		push(OP_CALL);
		push(reinterpret_cast<const IdentifierExpr*>(node->identifier.adr)->id);

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
	assert(node->attr->data.is<VarAttr>() || node->attr->data.is<ArgAttr>());

	if (node->attr->data.is<VarAttr>()) {
		push(OP_LOAD);
		push(node->attr->data.get<VarAttr>().slot);
	} else if (node->attr->data.is<ArgAttr>()) {
		push(OP_LOAD_STACK);
		push(node->attr->data.get<ArgAttr>().index);
	} else {
		LOG_ERR("Unimplemented identifier attribute loading");
		exit(1);
	}
}

void CodeGenerator::generate_store(const IdentifierExpr* node) {
	assert(node->attr->data.is<VarAttr>() || node->attr->data.is<ArgAttr>());

	if (node->attr->data.is<VarAttr>()) {
		push(OP_STORE);
		push(node->attr->data.get<VarAttr>().slot);
	} else if (node->attr->data.is<ArgAttr>()) {
		push(OP_STORE_STACK);
		push(node->attr->data.get<ArgAttr>().index);
	} else {
		LOG_ERR("Unimplemented identifier attribute loading");
		exit(1);
	}
}

} // namespace scr
