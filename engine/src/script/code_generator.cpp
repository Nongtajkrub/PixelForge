#include "code_generator.hpp"
#include "ast.hpp"

#include <cassert>

namespace scr {

bool CodeGenerator::generate() {
	for (const auto& node : this->ast) {
		if (!handle_node(node)) {
			return false;
		}
	}
	return true;
}

void CodeGenerator::output_inst(std::ostream& stream) {
	;;
}

bool CodeGenerator::handle_node(const ASTNode& ast_node) {
	return true;
}

} // namespace scr
