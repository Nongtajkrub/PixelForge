#include "code_generator.hpp"
#include "ast.hpp"
#include "symbol_table.hpp"
#include "vm_def.h"

#include <cassert>
#include <utility>

namespace scr {

bool CodeGenerator::generate() {
	for (const auto& node : this->ast) {
		if (!handle_node(node)) {
			return false;
		}
	}
	return true;
}

void CodeGenerator::output_code(std::ostream& stream) {
	for (u32 i = 0; i < this->code.size(); i++) {
		const auto op = static_cast<opcode_t>(this->code[i]);

		stream << op_to_str(op);
		if (op_have_operand(op)) {
			i++;
			assert(i < this->code.size());
			stream << ": " << this->code[i]; 
		}
		stream << '\n'; 
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
	case ASTNodeKind::BINARY:
		TODO();
		break;
	case ASTNodeKind::CALL:
		TODO();
		break;
	case ASTNodeKind::RANGE:
		TODO();
		break;
	default:
		LOG_ERR("Node is not an expression.");
		std::unreachable();
	}
}

} // namespace scr
