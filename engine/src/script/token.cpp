#include "token.hpp"

namespace scr {

const char* Token::kind_as_str() const {
	switch (this->kind) {
		case TokenKind::LEFT_BRACE: return "LEFT_BRACE";
        case TokenKind::RIGHT_BRACE: return "RIGHT_BRACE";
        case TokenKind::LEFT_BRACKET: return "LEFT_BRACKET";
        case TokenKind::RIGHT_BRACKET: return "RIGHT_BRACKET";
        case TokenKind::COMMA: return "COMMA";
        case TokenKind::MINUS: return "MINUS";
        case TokenKind::PLUS: return "PLUS";
        case TokenKind::SLASH: return "SLASH";
        case TokenKind::STAR: return "STAR";
        case TokenKind::EQUAL: return "EQUAL";
        case TokenKind::SEMICOLON: return "SEMICOLON";
        case TokenKind::BANG: return "BANG";
        case TokenKind::GREATER: return "GREATER";
        case TokenKind::LESS: return "LESS";

        // Multi-character tokens
        case TokenKind::DOUBLE_EQUAL: return "DOUBLE_EQUAL";
        case TokenKind::BANG_EQUAL: return "BANG_EQUAL";
        case TokenKind::GREATER_EQUAL: return "GREATER_EQUAL";
        case TokenKind::LESS_EQUAL: return "LESS_EQUAL";

        // Literals
        case TokenKind::IDENTIFIER: return "IDENTIFIER";
        case TokenKind::STRING: return "STRING";
        case TokenKind::NUMBER: return "NUMBER";

        // Keywords
        case TokenKind::AND: return "AND";
        case TokenKind::OR: return "OR";
        case TokenKind::IF: return "IF";
        case TokenKind::ENDIF: return "ENDIF";
        case TokenKind::ELSE: return "ELSE";
        case TokenKind::TRUE: return "TRUE";
        case TokenKind::FALSE: return "FALSE";
        case TokenKind::WHILE: return "WHILE";
        case TokenKind::FOR: return "FOR";
        case TokenKind::RETURN: return "RETURN";
        case TokenKind::PASS: return "PASS";
        case TokenKind::VAR: return "VAR";
        case TokenKind::FUNC: return "FUNC";
        case TokenKind::ENDFUNC: return "ENDFUNC";
        case TokenKind::PRINT: return "PRINT";
    }

    return "UNKNOWN_TOKEN";
}
bool Token::is_arithmetic_operator() const {
	switch (this->kind) {
	case TokenKind::MINUS:
	case TokenKind::PLUS:
	case TokenKind::SLASH:
	case TokenKind::STAR:
		return true;
	default:
		return false;
	}
}

} // namespace scr
