#include "code_generator.hpp"
#include "ast.hpp"
#include "symbol_table.hpp"
#include "token.hpp"
#include "vm_def.h"

#include <cassert>
#include <cstddef>
#include <cstdlib>

namespace scr {

bool CodeGenerator::generate() {
	push(OP_BEGIN);
	for (const auto& node : this->ast) {
		if (!handle_node(node)) {
			return false;
		}
	}
	push(OP_END);
	return  true;
}

static const char* label_to_str(Label label) {
	switch (label) {
	case Label::HOLE: return "HOLE"; 
	case Label::THEN_BRANCH: return "THEN_BRANCH";
	case Label::ELSE_BRANCH: return "ELSE_BRANCH";
	case Label::IF_END: return "IF_END";
	}
}

void CodeGenerator::output_code(std::ostream& stream) {
	for (size_t i = 0; i < this->code.size(); i++) {
		const auto& entry = this->code[i];

		if (entry.kind == CodeEntryKind::INSTRUCTION) {
			const auto op = static_cast<opcode_t>(entry.get_inst());

			stream << op_to_str(op); 

			if (op_have_operand(op)) {
				assert(i + 1 < this->code.size());
				const auto& operand_entry = this->code[++i];

				stream << ": ";
				if (operand_entry.kind == CodeEntryKind::INSTRUCTION) {
					stream << operand_entry.get_inst();
				} else {
					stream << label_to_str(operand_entry.get_label());
				}
			}
		} else if (entry.kind == CodeEntryKind::LABEL) {
			stream << "    " << label_to_str(std::get<Label>(entry.data));
		}

		stream << "\n";
	}
}

bool CodeGenerator::handle_node(const ASTNode& node) {
	switch (*node.adr) {
	case  ASTNodeKind::VAR_DECLARATION:
		handle_var_declaration(
			reinterpret_cast<const VarDeclarationStmt*>(node.adr));
		break;
	case ASTNodeKind::ASSIGN:
		handle_assign_stmt(reinterpret_cast<const AssignStmt*>(node.adr));
		break;
	case ASTNodeKind::IF:
		handle_if_stmt(reinterpret_cast<const IfStmt*>(node.adr));
		break;
	default:
		break;
	}

	return true;
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

void CodeGenerator::handle_if_stmt(const IfStmt* node) {
	handle_expr(node->expr);

	push(OP_JMP_FALSE);
	push(Label::ELSE_BRANCH);

	// Handle then branch.
	push(Label::THEN_BRANCH);
	const auto then_branch =
		reinterpret_cast<const BlockStmt*>(node->then_branch.adr);
	for (const auto& then_node : then_branch->block) {
		handle_node(then_node);
	}

	// Push jump instruction to avoid executing else branch after then branch.
	push(OP_JMP);
	push(Label::IF_END);

	push(Label::ELSE_BRANCH);

	// Handle else branch if exist.
	if (node->else_branch) {
		const auto else_branch =
			reinterpret_cast<const BlockStmt*>((*node->else_branch).adr);
		for (const auto& else_node : else_branch->block) {
			handle_node(else_node);
		}
	} 

	push(Label::IF_END);
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
		const auto node = reinterpret_cast<const IdentifierExpr*>(expr.adr);

		if (node->attr->kind == IdenKind::VAR) {
			generate_load(node);
		}

		break;
	}
	case ASTNodeKind::DOT:
		TODO();
		break;
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
	case ASTNodeKind::CALL:
		TODO();
		break;
	case ASTNodeKind::RANGE:
		TODO();
		break;
	default:
		LOG_ERR("Node is not an expression.");
		exit(1);
	}
}

} // namespace scr
