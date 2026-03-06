#include "ast.hpp"

namespace scr {

const char* ASTNode::kind_as_str() const {
    switch (*this->adr) {
		case ASTNodeKind::NOP: return "NOP";
        case ASTNodeKind::VAR_DECLARATION: return "VAR_DECLARATION";
        case ASTNodeKind::FUNC_DECLARATION: return "FUNC_DECLARATION";
        case ASTNodeKind::FUNC_ARGUMENTS: return "FUNC_ARGUMENTS";
        case ASTNodeKind::BINARY: return "BINARY";
		case ASTNodeKind::CALL: return "CALL";
        case ASTNodeKind::LITERAL: return "LITERAL";
        case ASTNodeKind::IDENTIFIER: return "IDENTIFIER";
        case ASTNodeKind::TYPE: return "TYPE";
    }

    return "UNKNOWN_AST_NODE";
}

void ast_output(std::ostream &stream, ASTNode root, const u32 level) {
	const auto indent = std::string(level, '\t');

	stream << indent << root.kind_as_str() << " ->\n";

	switch (*root.adr) {
	case ASTNodeKind::NOP:
		break;
	case ASTNodeKind::LITERAL: {
		auto node = reinterpret_cast<const PrimaryExpr*>(root.adr);
		stream << indent << "literal: " << *node->token.lexeme << '\n';
		break;
	}
	case ASTNodeKind::IDENTIFIER: {
		auto node = reinterpret_cast<const PrimaryExpr*>(root.adr);
		stream << indent << "identifier: " << *node->token.lexeme << '\n';
		break;
	}
	case ASTNodeKind::TYPE: {
		auto node = reinterpret_cast<const PrimaryExpr*>(root.adr);
		stream << indent << "type: " << *node->token.lexeme << '\n';
		break;
	}
	case ASTNodeKind::VAR_DECLARATION: {
		auto node = reinterpret_cast<const VarDeclarationStmt*>(root.adr);

		ast_output(stream, node->name, level + 1);

		ast_output(stream, node->type, level + 1);

		if (node->init) {
			ast_output(stream, *node->init, level + 1);
		}

		break;
	}
	case ASTNodeKind::FUNC_DECLARATION: {
		auto node = reinterpret_cast<const FuncDeclarationStmt*>(root.adr);

		ast_output(stream, node->name, level);

		stream << indent << "args (" << node->args.size() << "):\n";
		for (const auto& arg : node->args) {
			ast_output(stream, arg, level + 1);
		}

		stream << indent << "body: \n";
		for (const auto& stmt : node->body) {
			ast_output(stream, stmt, level + 1);
		}

		break;
	}
	case ASTNodeKind::FUNC_ARGUMENTS: {
		auto node = reinterpret_cast<const FuncArgument*>(root.adr);

		ast_output(stream, node->name, level + 1);
		ast_output(stream, node->type, level + 1);

		break;
	}
	case ASTNodeKind::BINARY: {
		auto node = reinterpret_cast<const BinaryExpr*>(root.adr);

		stream << indent << "left: \n";
		ast_output(stream, node->left, level + 1);

		stream << indent << "operator: " << node->op.kind_as_str() << '\n';

		stream << indent << "right: \n";
		ast_output(stream, node->right, level + 1);

		break;
	}
	case ASTNodeKind::CALL: {
		auto node = reinterpret_cast<const CallExpr*>(root.adr);

		ast_output(stream, node->name, level);

		stream << indent << "args (" << node->args.size() << "):\n";
		for (const auto& arg : node->args) {
			ast_output(stream, arg, level + 1);
		}

		break;
	}
	default:
		stream << "UNKNOW\n";
		return;
}
}

} // namespace scr
