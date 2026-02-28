#include "diagnostic.hpp"

#include <ostream>
#include <string_view>

namespace scr {

std::string_view Diagnostic::resolve_msg() const {
    switch (this->kind) {
    case DiagnosticKind::UNEXPECTED_TOKEN: return "unexpected token";
    case DiagnosticKind::EXPECTED_TOKEN: return "expected token";
    case DiagnosticKind::EXPECTED_IDENTIFIER: return "expected identifier";
    case DiagnosticKind::EXPECTED_EXPRESSION: return "expected expression";
    case DiagnosticKind::EXPECTED_TYPE: return "expected type";
	case DiagnosticKind::EXPECTED_LITERAL: return "expected literal expression";
    case DiagnosticKind::EXPECTED_SEMICOLON: return "expected ';'";
    case DiagnosticKind::EXPECTED_RIGHT_PAREN: return "expected ')'";
    case DiagnosticKind::EXPECTED_RIGHT_BRACE: return "expected '}'";
    case DiagnosticKind::EXPECTED_LEFT_BRACKET: return "expected '['";
	case DiagnosticKind::EXPECTED_RIGHT_BRACKET: return "expected ']'";
	case DiagnosticKind::EXPECTED_NUMBER: return "expected a number";
    case DiagnosticKind::INVALID_ASSIGNMENT_TARGET: 
		return "invalid assignment target";
    case DiagnosticKind::TOO_MANY_ARGUMENTS: return "too many arguments";
    case DiagnosticKind::UNEXPECTED_END_OF_FILE: return "unexpected end of file";
    case DiagnosticKind::UNKNOWN_IDENTIFIER: return "unknown identifier";
    case DiagnosticKind::REDECLARED_IDENTIFIER: return "redeclared identifier";
    case DiagnosticKind::TYPE_MISMATCH: return "type mismatch";
    case DiagnosticKind::INVALID_BINARY_OPERANDS: 
		return "invalid operands to binary operator";
    case DiagnosticKind::CANNOT_ASSIGN_TO_CONST: 
		return "cannot assign to const variable";
    case DiagnosticKind::UNUSED_VARIABLE: return "unused variable";
    case DiagnosticKind::UNREACHABLE_CODE: return "unreachable code";
    case DiagnosticKind::SHADOWED_VARIABLE: 
		return "declaration shadows previous variable";
    }

	return "Error message not implemented.";
}

void Diagnostic::emit(std::ostream& stream) const {
	stream 
		<< resolve_msg() 
		<< " at line " 
		<< this->location.line
		<< '\n';
}

} // namespace scr
