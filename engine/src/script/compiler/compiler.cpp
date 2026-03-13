#include "compiler.hpp"
#include "../bytecode/bytecode.hpp"

namespace scr {

bool Compiler::compile() {
	for (const auto node : this->nodes) {
		switch (*node.adr) {
		case ASTNodeKind::VAR_DECLARATION:
			if (!comp_var_declaration(node)) return false;
			break;
		default:
			return false;
		}
	}

	return true;
}

bool Compiler::comp_var_declaration(ASTNode raw_node) {
	auto node = reinterpret_cast<const VarDeclarationStmt*>(raw_node.adr);

	if (node->init) {
		comp_expr(*node->init);
	}

	return true;
}

bool Compiler::comp_expr(ASTNode raw_node) {
}

} // namespace scr
