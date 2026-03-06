#include "diagnostic.hpp"

#include <ostream>
#include <string_view>

namespace scr {

std::string_view Diagnostic::resolve_msg() const {
    switch (this->kind) {
		case DiagnosticKind::UNEXPECTED_EOF: return "Unexpected EOF.";
		case DiagnosticKind::UNEXPECTED_TOKEN: return "Unexpected token.";
		case DiagnosticKind::EXPECTED_SEMICOLON: return "expected semicolon";
		case DiagnosticKind::EXPECTED_COLON: return "Expected colon";
		case DiagnosticKind::EXPECTED_IDENTIFIER: return "Expected identifier";
		case DiagnosticKind::EXPECTED_LEFT_PAREN: return "Expected left paren"; 
		case DiagnosticKind::EXPECTED_RIGHT_PAREN: return "Expected right paren";
		case DiagnosticKind::EXPECTED_COMMA: return "Expected comma"; 
    }

	return "Error message not implemented.";
}

void Diagnostic::emit(std::ostream& stream) const {
	stream 
		<< resolve_msg() 
		<< " at " 
		<< this->location.row
		<< ":"
		<< this->location.col
		<< '\n';
}

} // namespace scr
