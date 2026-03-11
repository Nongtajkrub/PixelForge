#include "token.hpp"

namespace scr {

const char* Token::kind_as_str() const {
	switch (this->kind) {
		case TokenKind::LEFT_BRACE: return "LEFT_BRACE";
        case TokenKind::RIGHT_BRACE: return "RIGHT_BRACE";
        case TokenKind::LEFT_BRACKET: return "LEFT_BRACKET";
        case TokenKind::RIGHT_BRACKET: return "RIGHT_BRACKET";
        case TokenKind::LEFT_PAREN: return "LEFT_PAREN";
        case TokenKind::RIGHT_PAREN: return "RIGHT_PAREN";
        case TokenKind::COMMA: return "COMMA";
        case TokenKind::MINUS: return "MINUS";
        case TokenKind::PLUS: return "PLUS";
        case TokenKind::SLASH: return "SLASH";
        case TokenKind::STAR: return "STAR";
        case TokenKind::EQUAL: return "EQUAL";
        case TokenKind::SEMICOLON: return "SEMICOLON";
        case TokenKind::COLON: return "COLON";
        case TokenKind::BANG: return "BANG";
        case TokenKind::GREATER: return "GREATER";
        case TokenKind::LESS: return "LESS";

        // Multi-character tokens
        case TokenKind::DOUBLE_EQUAL: return "DOUBLE_EQUAL";
        case TokenKind::BANG_EQUAL: return "BANG_EQUAL";
        case TokenKind::GREATER_EQUAL: return "GREATER_EQUAL";
        case TokenKind::LESS_EQUAL: return "LESS_EQUAL";
        case TokenKind::ARROW: return "ARROW";

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
		case TokenKind::SELF: return "SELF";
        case TokenKind::WHILE: return "WHILE";
        case TokenKind::FOR: return "FOR";
        case TokenKind::RETURN: return "RETURN";
        case TokenKind::PASS: return "PASS";
		case TokenKind::LET: return "LET";
        case TokenKind::FUNC: return "FUNC";
        case TokenKind::ENDFUNC: return "ENDFUNC";
		
		case TokenKind::DIRECT_SPRITE: return "DIRECT_SPRITE";
		case TokenKind::DIRECT_USE: return "DIRECT_USE";

		case TokenKind::COMMAND: return "COMMAND";
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

bool Token::is_directive() const {
	switch (this->kind) {
	case TokenKind::DIRECT_SPRITE:
	case TokenKind::DIRECT_USE:
		return true;
	default:
		return false;
	}
}
} // namespace scr
