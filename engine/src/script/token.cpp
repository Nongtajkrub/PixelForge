#include "token.hpp"
#include "diagnostic.hpp"
#include "../core/io/log.hpp"

#include <initializer_list>
#include <utility>

namespace scr {

bool TokenStream::ensure_not_eof() {
	if (is_eof()) {
		Diagnostic(DiagnosticKind::UNEXPECTED_EOF, prev())
			.emit(this->err_stream);
		return false;
	}
	return true;
}

bool TokenStream::expect(TokenKind kind) {
	if (!ensure_not_eof()) {
		return false;
	}

	if (match_kind(kind)) {
		return true;
	} else {
		Diagnostic(resolve_diag_expect_kind(kind), prev())
			.emit(this->err_stream);
		return false;
	}
}

bool TokenStream::expect_peek(TokenKind kind) {
	if (!ensure_not_eof()) {
		return false;
	}

	if (peek().kind == kind) {
		return true;
	} else {
		Diagnostic(resolve_diag_expect_kind(kind), peek())
			.emit(this->err_stream);
		return false;
	}
}

void MutTokenStream::replace_and_insert(std::initializer_list<Token> v) {
	auto it = this->source.begin() + this->pos;

	*it = *v.begin();
	this->source.insert(it + 1, v.begin() + 1, v.end());
	this->pos += v.size();
}

const char* token_kind_as_str(TokenKind kind) {
	switch (kind) {
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
	case TokenKind::INTEGER: return "NUMBER";
	case TokenKind::FLOAT: return "FLOAT";

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
	case TokenKind::GET: return "GET";

	case TokenKind::VOID_T: return "VOID_T";
	case TokenKind::INT_T: return "INT_T";
	case TokenKind::FLOAT_T: return "FLOAT_T";
	case TokenKind::BOOL_T: return "BOOL_T";
	case TokenKind::STRING_T: return "STRING_T";
	case TokenKind::SPRITE_T: return "SPRITE_T";
	
	case TokenKind::DIRECT_SPRITE: return "DIRECT_SPRITE";
	case TokenKind::DIRECT_USE: return "DIRECT_USE";
	case TokenKind::DIRECT_UPDATE: return "DIRECT_UPDATE";
	case TokenKind::DIRECT_COLLIDE: return "DIRECT_COLLIDE";

	case TokenKind::COMMAND: return "COMMAND";
    }

    return "UNKNOWN_TOKEN";
}

TokenKind token_kind_as_type(TokenKind kind) {
	switch (kind) {
	case TokenKind::STRING: return TokenKind::STRING_T;
	case TokenKind::INTEGER: return TokenKind::INT_T;
	case TokenKind::FLOAT: return TokenKind::FLOAT_T;
	default: 
		LOG_ERR("This TokenKind can not be interpret as a type");
		std::unreachable();
	}
}

bool is_arithmetic_operator(TokenKind kind) {
	switch (kind) {
	case TokenKind::MINUS:
	case TokenKind::PLUS:
	case TokenKind::SLASH:
	case TokenKind::STAR:
		return true;
	default:
		return false;
	}
}

bool token_is_directive(TokenKind kind) {
	switch (kind) {
	case TokenKind::DIRECT_SPRITE:
	case TokenKind::DIRECT_USE:
		return true;
	default:
		return false;
	}
}

bool token_is_type(TokenKind kind) {
	switch (kind) {
	case TokenKind::VOID_T:
	case TokenKind::INT_T:
	case TokenKind::FLOAT_T:
	case TokenKind::BOOL_T:
	case TokenKind::STRING_T:
	case TokenKind::SPRITE_T:
		return true;
	default:
		return false;
	}
}

} // namespace scr
