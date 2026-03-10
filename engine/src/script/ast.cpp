#include "ast.hpp"

namespace scr {

const char* ASTNode::kind_as_str() const {
    switch (*this->adr) {
		case ASTNodeKind::NOP: return "NOP";
        case ASTNodeKind::BLOCK: return "BLOCK";
        case ASTNodeKind::VAR_DECLARATION: return "VAR_DECLARATION";
		case ASTNodeKind::DIRECTIVE: return "DIRECTIVE";
        case ASTNodeKind::FUNC_DECLARATION: return "FUNC_DECLARATION";
        case ASTNodeKind::FUNC_ARGUMENTS: return "FUNC_ARGUMENTS";
        case ASTNodeKind::IF: return "IF";
        case ASTNodeKind::BINARY: return "BINARY";
		case ASTNodeKind::CALL: return "CALL";
        case ASTNodeKind::COMMAND: return "COMMAND";
        case ASTNodeKind::LITERAL: return "LITERAL";
        case ASTNodeKind::IDENTIFIER: return "IDENTIFIER";
        case ASTNodeKind::TYPE: return "TYPE";
		case ASTNodeKind::KEYWORD: return "KEYWORD";
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
	case ASTNodeKind::KEYWORD: {
		auto node = reinterpret_cast<const PrimaryExpr*>(root.adr);
		stream << indent << "keyword: " << node->token.kind_as_str() << '\n';
		break;
	}
	case ASTNodeKind::DIRECTIVE: {
		auto node = reinterpret_cast<const DirectiveStmt*>(root.adr);

		stream << "directive:\n";
		ast_output(stream, node->directive, level + 1);

		stream << "expr:\n";
		if (node->expr) {
			ast_output(stream, *node->expr, level + 1);
		}

		break;
	}
	case ASTNodeKind::BLOCK: {
		auto node = reinterpret_cast<const BlockStmt*>(root.adr);

		for (const auto node : node->block) {
			ast_output(stream, node, level + 1);
		}

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

		stream << indent << "return:\n";
		ast_output(stream, node->type, level + 1);

		stream << indent << "body:\n";
		ast_output(stream, node->body, level + 1);

		break;
	}
	case ASTNodeKind::FUNC_ARGUMENTS: {
		auto node = reinterpret_cast<const FuncArgument*>(root.adr);

		ast_output(stream, node->name, level + 1);
		ast_output(stream, node->type, level + 1);

		break;
	}
	case ASTNodeKind::IF: {
		auto node = reinterpret_cast<const IfStmt*>(root.adr);

		stream << indent << "condition:\n";
		ast_output(stream, node->expr, level + 1);

		stream << indent << "then:\n";
		ast_output(stream, node->then_branch, level + 1);

		if (node->else_branch) {
			stream << indent << "else:\n";
			ast_output(stream, *(node->else_branch), level + 1);
		}

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
	case ASTNodeKind::COMMAND: {
		auto node = reinterpret_cast<const CommandStmt*>(root.adr);

		stream << indent << "command:\n";
		ast_output(stream, node->command, level + 1);

		stream << indent << "target:\n";
		ast_output(stream, node->target, level + 1);

		stream << indent << "args (" << node->operands.size() << "):\n";
		for (const auto operand : node->operands) {
			ast_output(stream, operand, level + 1);
		}

		break;
	}
	default:
		stream << "UNKNOW\n";
		return;
}
}

} // namespace scr
