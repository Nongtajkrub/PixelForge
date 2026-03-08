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
	case DiagnosticKind::EXPECTED_COMMAND: return "Expected command"; 
	case DiagnosticKind::EXPECTED_NUMBER: return "Expected number"; 
    }

	return "Error message not implemented.";
}

DiagnosticKind resolve_diag_expect_kind(TokenKind kind) {
	switch (kind) {
	case TokenKind::SEMICOLON: return DiagnosticKind::EXPECTED_SEMICOLON;
	case TokenKind::COLON: return DiagnosticKind::EXPECTED_COLON;
	case TokenKind::IDENTIFIER: return DiagnosticKind::EXPECTED_IDENTIFIER;
	case TokenKind::LEFT_PAREN: return DiagnosticKind::EXPECTED_LEFT_PAREN;
	case TokenKind::RIGHT_PAREN: return DiagnosticKind::EXPECTED_RIGHT_PAREN;
	case TokenKind::COMMA: return DiagnosticKind::EXPECTED_COMMA;
	case TokenKind::NUMBER: return DiagnosticKind::EXPECTED_NUMBER;
	case TokenKind::CMD_UP:
	case TokenKind::CMD_DOWN:
	case TokenKind::CMD_RIGHT:
	case TokenKind::CMD_GOTO:
		return DiagnosticKind::EXPECTED_COMMAND;
	default:
		std::println("from: {}", static_cast<int>(kind));
		return DiagnosticKind::UNEXPECTED_TOKEN;
	}
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
