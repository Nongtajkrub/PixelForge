#include "diagnostic.hpp"

#include <ostream>
#include <string_view>

namespace scr {

std::string_view Diagnostic::resolve_msg() const {
    switch (this->kind) {
	case DiagnosticKind::FAIL_OPEN_SOURCE: return "Fail to open source file";
	case DiagnosticKind::UNEXPECTED_EOF: return "Unexpected EOF";
	case DiagnosticKind::UNEXPECTED_TOKEN: return "Unexpected token";
	case DiagnosticKind::EXPECTED_SEMICOLON: return "expected semicolon";
	case DiagnosticKind::EXPECTED_COLON: return "Expected colon";
	case DiagnosticKind::EXPECTED_IDENTIFIER: return "Expected identifier";
	case DiagnosticKind::EXPECTED_LEFT_PAREN: return "Expected left paren"; 
	case DiagnosticKind::EXPECTED_RIGHT_PAREN: return "Expected right paren";
	case DiagnosticKind::EXPECTED_ARROW: return "Expected arrow";
	case DiagnosticKind::EXPECTED_COMMA: return "Expected comma"; 
	case DiagnosticKind::EXPECTED_COMMAND: return "Expected command"; 
	case DiagnosticKind::EXPECTED_INTEGER: return "Expected integer"; 
	case DiagnosticKind::EXPECTED_FLOAT: return "Expected integer";
	case DiagnosticKind::EXPECTED_BOOL: return "Expected boolean";
	case DiagnosticKind::EXPECTED_STRING: return "Expected string";
	case DiagnosticKind::EXPECTED_SPRITE: return "Expected sprite";
	case DiagnosticKind::EXPECTED_SPRITE_DIRECT:
		return "Expected sprite directive";
	case DiagnosticKind::EXPECTED_TYPE: return "Expected type";
	case DiagnosticKind::EXPECTED_VALUE_TYPE: return "Expected value type";
	case DiagnosticKind::EXPECTED_PROPERTY: return "Expected property";
	case DiagnosticKind::EXPECTED_FUNCTION: return "Expected function";
	case DiagnosticKind::EXPECTED_RANGE_OP: return "Expected range operator";
	case DiagnosticKind::EXPECTED_RANGE_EXPR: return "Expected range expression";
	case DiagnosticKind::UNKNOWN_IDENTIFIER: return "Unknown identifier";
	case DiagnosticKind::UNTERMINATED_STRING: return "Unterminated string";
	case DiagnosticKind::TYPE_ERROR: return "Type error";
	case DiagnosticKind::TO_MANY_ARGUMENTS: return "To many arguments";
	case DiagnosticKind::JUMP_NOT_ALLOW: return "Jump statments not allow";
	case DiagnosticKind::FUNC_DECL_NOT_ALLOW: 
		return "Function declaration not allow";
	case DiagnosticKind::RETURN_NOT_ALLOW: return "Return not allow";
	case DiagnosticKind::VARIABLE_REDECLARATION: return "Varaible redeclaration";
	case DiagnosticKind::FUNC_EXPECTED_RETURN: return "Function expected return";
	case DiagnosticKind::FUNC_UNEXPECTED_RETURN:
		return "Function unexpected return";
    }

	return "Error message not implemented";
}

DiagnosticKind resolve_diag_expect_kind(TokenKind kind) {
	switch (kind) {
	case TokenKind::SEMICOLON: return DiagnosticKind::EXPECTED_SEMICOLON;
	case TokenKind::COLON: return DiagnosticKind::EXPECTED_COLON;
	case TokenKind::IDENTIFIER: return DiagnosticKind::EXPECTED_IDENTIFIER;
	case TokenKind::LEFT_PAREN: return DiagnosticKind::EXPECTED_LEFT_PAREN;
	case TokenKind::RIGHT_PAREN: return DiagnosticKind::EXPECTED_RIGHT_PAREN;
	case TokenKind::ARROW: return DiagnosticKind::EXPECTED_ARROW;
	case TokenKind::COMMA: return DiagnosticKind::EXPECTED_COMMA;
	case TokenKind::INTEGER: return DiagnosticKind::EXPECTED_INTEGER;
	case TokenKind::INT_T: return DiagnosticKind::EXPECTED_INTEGER;
	case TokenKind::FLOAT_T: return DiagnosticKind::EXPECTED_FLOAT;
	case TokenKind::BOOL_T: return DiagnosticKind::EXPECTED_BOOL;
	case TokenKind::STRING_T: return DiagnosticKind::EXPECTED_STRING; 
	case TokenKind::SPRITE_T: return DiagnosticKind::EXPECTED_SPRITE; 
	case TokenKind::COMMAND: return DiagnosticKind::EXPECTED_COMMAND;
	case TokenKind::DIRECT_SPRITE: return DiagnosticKind::EXPECTED_SPRITE_DIRECT;
	case TokenKind::RANGE_OP: return DiagnosticKind::EXPECTED_RANGE_OP;
	default: return DiagnosticKind::UNEXPECTED_TOKEN;
	}
}

void Diagnostic::emit(std::ostream& stream) const {
	stream << resolve_msg(); 

	if (this->location) {
		stream 
			<< " at " 
			<< (*this->location).row
			<< ":"
			<< (*this->location).col;
	}

	stream << '\n'; 
}

} // namespace scr
