#include "code_generator.hpp"
#include "ast.hpp"
#include "bytecode.hpp"
#include "token.hpp"

#include <cassert>
#include <ostream>
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

bool CodeGenerator::handle_node(const ASTNode& ast_node) {
	switch (*ast_node.adr) {
	case ASTNodeKind::VAR_DECLARATION:
		return handle_var_declaration(ast_node);
	case ASTNodeKind::LITERAL:
		return handle_literal(ast_node);
	case ASTNodeKind::IDENTIFIER:
		return handle_identifier(ast_node);
	default:
		return false;
	}
}

void CodeGenerator::output_inst(std::ostream& stream) {
	for (const auto& inst : this->instructions) {
		stream << op_to_str(inst.op);
		if (inst.operand) {
			stream << ": " << *(inst.operand);
		}
		stream << '\n';
	}
}

bool CodeGenerator::handle_var_declaration(const ASTNode& ast_node) {
	const auto node = reinterpret_cast<const VarDeclarationStmt*>(ast_node.adr);

	if (node->init) {
		if (!handle_node(*(node->init))) {
			return false;
		}
	}

	if (!handle_node(node->identifier)) {
		return false;
	}

	return true;
}

bool CodeGenerator::handle_literal(const ASTNode& ast_node) {
	assert(*(ast_node.adr) == ASTNodeKind::LITERAL);

	this->instructions.emplace_back(
		OpCode::CONST,
		this->const_pool.intern_const(const_from_literal(ast_node)));

	return true;
}

bool CodeGenerator::handle_identifier(const ASTNode& ast_node) {
	return true;
}

Const CodeGenerator::const_from_literal(const ASTNode& ast_node) {
	assert(*(ast_node.adr) == ASTNodeKind::LITERAL);
	const auto node = reinterpret_cast<const PrimaryExpr*>(ast_node.adr);

	// Create a new constant entry with the resolve type.
	auto entry = Const(token_to_type(node->token.kind));

	switch (node->token.kind) {
	case TokenKind::INTEGER:
		entry.data.int_const = std::stoi(*(node->token.lexeme));
		break;
	case TokenKind::FLOAT:
		entry.data.float_const = std::stof(*(node->token.lexeme));
		break;
	case TokenKind::TRUE:
		entry.data.bool_const = true;
		break;
	case TokenKind::FALSE:
		entry.data.bool_const = false;
		break;
	case TokenKind::STRING:
		TODO();
		break;
	default:
		LOG_ERR("Other token kinds would not be consider as literals.");
		std::unreachable();
	}

	return entry;
}

} // namespace scr
