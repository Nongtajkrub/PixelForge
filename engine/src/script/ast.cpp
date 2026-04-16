#include "ast.hpp"
#include <cassert>

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
	case ASTNodeKind::BREAK: return "BREAK";
	case ASTNodeKind::CONTINUE: return "CONTINUE";
	case ASTNodeKind::RETURN: return "RETURN";
    }

    return "UNKNOWN_AST_NODE";
}

std::vector<const BlockStmt*> ASTNode::get_child_blocks() const {
	switch (*this->adr) {
	case ASTNodeKind::BLOCK:
		return {reinterpret_cast<const BlockStmt*>(this->adr)};
	case ASTNodeKind::FOR_LOOP: {
		const auto node = reinterpret_cast<const ForLoopStmt*>(this->adr);
		return {reinterpret_cast<const BlockStmt*>(node->block.adr)};
	}
	case ASTNodeKind::FUNC_DECLARATION: {
		const auto node = reinterpret_cast<const FuncDeclarationStmt*>(this->adr);
		return {reinterpret_cast<const BlockStmt*>(node->body.adr)};
	}
	case ASTNodeKind::IF: {
		const auto node = reinterpret_cast<const IfStmt*>(this->adr);

		if (node->else_branch) {
			return {
				reinterpret_cast<const BlockStmt*>(node->then_branch.adr),
				reinterpret_cast<const BlockStmt*>((*node->else_branch).adr)};
		} else {
			return {reinterpret_cast<const BlockStmt*>(node->then_branch.adr)};
		}
	}
	default:
		return {};
	}
}

void ASTNode::output(std::ostream &stream, const u32 level) const {
	const auto indent = std::string(level, '\t');

	stream << indent << kind_as_str() << " ->\n";

	switch (*this->adr) {
	case ASTNodeKind::NOP:
	case ASTNodeKind::BREAK:
	case ASTNodeKind::CONTINUE:
		break;
	case ASTNodeKind::LITERAL: {
		auto node = reinterpret_cast<const LiteralExpr*>(this->adr);
		stream << indent << "literal: " << node->index << '\n';
		break;
	}
	case ASTNodeKind::IDENTIFIER: {
		auto node = reinterpret_cast<const IdentifierExpr*>(this->adr);
		stream << indent << "identifier: " << node->id << '\n';
		break;
	}
	case ASTNodeKind::TYPE: {
		auto node = reinterpret_cast<const PrimaryExpr*>(this->adr);
		stream 
			<< indent 
			<< "type: " <<  token_kind_as_str(node->token.kind) << '\n';
		break;
	}
	case ASTNodeKind::KEYWORD: {
		auto node = reinterpret_cast<const PrimaryExpr*>(this->adr);

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
		auto node = reinterpret_cast<const DirectiveStmt*>(this->adr);

		stream << indent << "directive:\n";
		node->directive.output(stream, level + 1);

		stream << indent << "identifier:\n";
		node->identifier.output(stream, level + 1);

		break;
	}
	case ASTNodeKind::RETURN: {
		auto node = reinterpret_cast<const ReturnStmt*>(this->adr);

		if (node->expr) {
			stream << "expr:\n";
			(*node->expr).output(stream, level + 1);
		}

		break;
	}
	case ASTNodeKind::DOT: {
		auto node = reinterpret_cast<const DotExpr*>(this->adr);

		stream << indent << "identifier:\n";
		node->object.output(stream, level + 1);

		stream << indent << "property:\n";
		stream << indent << '\t' << node->property << '\n';
	}
	case ASTNodeKind::BLOCK: {
		auto node = reinterpret_cast<const BlockStmt*>(this->adr);

		for (const auto node : node->block) {
			node.output(stream, level + 1);
		}

		break;
	}
	case ASTNodeKind::VAR_DECLARATION: {
		auto node = reinterpret_cast<const VarDeclarationStmt*>(this->adr);

		stream << indent << "identifier:\n";
		node->identifier.output(stream, level + 1);

		stream << indent << "type:\n";
		node->type.output(stream, level + 1);

		if (node->init) {
			stream << indent << "init:\n";
			(*node->init).output(stream, level + 1);
		}

		break;
	}
	case ASTNodeKind::FUNC_DECLARATION: {
		auto node = reinterpret_cast<const FuncDeclarationStmt*>(this->adr);

		node->identifier.output(stream, level);

		stream << indent << "args (" << node->args.size() << "):\n";
		for (const auto& arg : node->args) {
			arg.output(stream, level + 1);
		}

		stream << indent << "return:\n";
		node->type.output(stream, level + 1);

		stream << indent << "body:\n";
		node->body.output(stream, level + 1);

		break;
	}
	case ASTNodeKind::FUNC_ARGUMENTS: {
		auto node = reinterpret_cast<const FuncArgument*>(this->adr);

		node->identifier.output(stream, level + 1);
		node->type.output(stream, level + 1);

		break;
	}
	case ASTNodeKind::ASSIGN: {
		auto node = reinterpret_cast<const AssignStmt*>(this->adr);

		node->identifier.output(stream, level + 1);
		node->expr.output(stream, level + 1);

		break;
	}
	case ASTNodeKind::IF: {
		auto node = reinterpret_cast<const IfStmt*>(this->adr);

		stream << indent << "condition:\n";
		node->expr.output(stream, level + 1);

		stream << indent << "then:\n";
		node->then_branch.output(stream, level + 1);

		if (node->else_branch) {
			stream << indent << "else:\n";
			(*node->else_branch).output(stream, level + 1);
		}

		break;
	}
	case ASTNodeKind::RANGE: {
		auto node = reinterpret_cast<const RangeExpr*>(this->adr);

		stream << indent << "begin\n";
		node->begin.output(stream, level + 1);


		stream << indent << "end\n";
		node->end.output(stream, level + 1);

		if (node->step) {
			stream << indent << "step\n";
			(*node->step).output(stream, level + 1);
		}

		break;
	}
	case ASTNodeKind::FOR_LOOP: {
		auto node = reinterpret_cast<const ForLoopStmt*>(this->adr);

		if (node->it) {
			stream << indent << "iterator:\n";
			(*node->it).output(stream, level + 1);
		}

		stream << indent << "range:\n";
		node->range.output(stream, level + 1);

		stream << indent << "code block:\n";
		node->block.output(stream, level + 1);

		break;
	}
	case ASTNodeKind::BINARY: {
		auto node = reinterpret_cast<const BinaryExpr*>(this->adr);

		stream << indent << "left: \n";
		node->left.output(stream, level + 1);

		stream 
			<< indent 
			<< "operator: " << token_kind_as_str(node->op.kind) << '\n';

		stream << indent << "right: \n";
		node->right.output(stream, level + 1);

		break;
	}
	case ASTNodeKind::CALL: {
		auto node = reinterpret_cast<const CallExpr*>(this->adr);

		node->identifier.output(stream, level);

		stream << indent << "args (" << node->args.size() << "):\n";
		for (const auto& arg : node->args) {
			arg.output(stream, level + 1);
		}

		break;
	}
	case ASTNodeKind::COMMAND: {
		auto node = reinterpret_cast<const CommandStmt*>(this->adr);

		stream << indent << "command:\n";
		stream << indent  << '\t' << node->id;

		stream << indent << "target:\n";
		node->target.output(stream, level + 1);

		stream << indent << "args (" << node->args.size() << "):\n";
		for (const auto operand : node->args) {
			operand.output(stream, level + 1);
		}

		break;
	}
	default:
		stream << "UNKNOW\n";
		return;
	}
}

} // namespace scr
