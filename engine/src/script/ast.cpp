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
        case ASTNodeKind::ASSIGN: return "ASSIGN";
        case ASTNodeKind::IF: return "IF";
        case ASTNodeKind::FOR_LOOP: return "FOR_LOOP";
        case ASTNodeKind::LOOP: return "LOOP";
        case ASTNodeKind::BINARY: return "BINARY";
		case ASTNodeKind::CALL: return "CALL";
		case ASTNodeKind::DOT: return "DOT";
		case ASTNodeKind::RANGE: return "RANGE";
        case ASTNodeKind::COMMAND: return "COMMAND";
        case ASTNodeKind::LITERAL: return "LITERAL";
        case ASTNodeKind::IDENTIFIER: return "IDENTIFIER";
        case ASTNodeKind::TYPE: return "TYPE";
		case ASTNodeKind::KEYWORD: return "KEYWORD";
		case ASTNodeKind::SELF: return "SELF";
		case ASTNodeKind::BREAK: return "BREAK";
		case ASTNodeKind::CONTINUE: return "CONTINUE";
		case ASTNodeKind::RETURN: return "RETURN";
    }

    return "UNKNOWN_AST_NODE";
}

void ast_output(std::ostream &stream, ASTNode root, const u32 level) {
	const auto indent = std::string(level, '\t');

	stream << indent << root.kind_as_str() << " ->\n";

	switch (*root.adr) {
	case ASTNodeKind::NOP:
	case ASTNodeKind::SELF:
	case ASTNodeKind::BREAK:
	case ASTNodeKind::CONTINUE:
		break;
	case ASTNodeKind::LITERAL: {
		auto node = reinterpret_cast<const PrimaryExpr*>(root.adr);
		stream << indent << "literal: " << *node->token.lexeme << '\n';
		break;
	}
	case ASTNodeKind::IDENTIFIER: {
		auto node = reinterpret_cast<const IdentifierExpr*>(root.adr);
		stream << indent << "identifier: " << node->id << '\n';
		break;
	}
	case ASTNodeKind::TYPE: {
		auto node = reinterpret_cast<const PrimaryExpr*>(root.adr);
		stream 
			<< indent 
			<< "type: " <<  token_kind_as_str(node->token.kind) << '\n';
		break;
	}
	case ASTNodeKind::KEYWORD: {
		auto node = reinterpret_cast<const PrimaryExpr*>(root.adr);

		// Some keyword kind like COMMAND have a lexeme.
		stream 
			<< indent 
			<< "keyword: " 
			<< ((node->token.lexeme) ?
					*node->token.lexeme : token_kind_as_str(node->token.kind)) 
			<< '\n';

		break;
	}
	case ASTNodeKind::DIRECTIVE: {
		auto node = reinterpret_cast<const DirectiveStmt*>(root.adr);

		stream << indent << "directive:\n";
		ast_output(stream, node->directive, level + 1);

		stream << indent << "identifier:\n";
		ast_output(stream, node->identifier, level + 1);

		break;
	}
	case ASTNodeKind::RETURN: {
		auto node = reinterpret_cast<const ReturnStmt*>(root.adr);

		if (node->expr) {
			stream << "expr:\n";
			ast_output(stream, *node->expr, level + 1);
		}

		break;
	}
	case ASTNodeKind::DOT: {
		auto node = reinterpret_cast<const DotExpr*>(root.adr);

		stream << indent << "identifier:\n";
		ast_output(stream, node->object, level + 1);

		stream << indent << "property:\n";
		stream << indent << '\t' << node->property << '\n';
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

		stream << indent << "identifier:\n";
		ast_output(stream, node->identifier, level + 1);

		stream << indent << "type:\n";
		ast_output(stream, node->type, level + 1);

		if (node->init) {
			stream << indent << "init:\n";
			ast_output(stream, *node->init, level + 1);
		}

		break;
	}
	case ASTNodeKind::FUNC_DECLARATION: {
		auto node = reinterpret_cast<const FuncDeclarationStmt*>(root.adr);

		ast_output(stream, node->identifier, level);

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

		ast_output(stream, node->identifier, level + 1);
		ast_output(stream, node->type, level + 1);

		break;
	}
	case ASTNodeKind::ASSIGN: {
		auto node = reinterpret_cast<const AssignStmt*>(root.adr);

		ast_output(stream, node->identifier, level + 1);
		ast_output(stream, node->expr, level + 1);

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
	case ASTNodeKind::RANGE: {
		auto node = reinterpret_cast<const RangeExpr*>(root.adr);

		stream << indent << "begin\n";
		ast_output(stream, node->begin, level + 1);

		stream << indent << "end\n";
		ast_output(stream, node->end, level + 1);

		if (node->step) {
			stream << indent << "step\n";
			ast_output(stream, *node->step, level + 1);
		}

		break;
	}
	case ASTNodeKind::FOR_LOOP: {
		auto node = reinterpret_cast<const ForLoopStmt*>(root.adr);

		if (node->it) {
			stream << indent << "iterator:\n";
			ast_output(stream, *node->it, level + 1);
		}

		stream << indent << "range:\n";
		ast_output(stream, node->range, level + 1);

		stream << indent << "code block:\n";
		ast_output(stream, node->block, level + 1);

		break;
	}
	case ASTNodeKind::BINARY: {
		auto node = reinterpret_cast<const BinaryExpr*>(root.adr);

		stream << indent << "left: \n";
		ast_output(stream, node->left, level + 1);

		stream 
			<< indent 
			<< "operator: " << token_kind_as_str(node->op.kind) << '\n';

		stream << indent << "right: \n";
		ast_output(stream, node->right, level + 1);

		break;
	}
	case ASTNodeKind::CALL: {
		auto node = reinterpret_cast<const CallExpr*>(root.adr);

		ast_output(stream, node->identifier, level);

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
