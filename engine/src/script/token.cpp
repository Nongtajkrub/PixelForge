#include "token.hpp"

#include "../core/cplusplus/io/log.hpp"
#include "diagnostic.hpp"
#include "specs.h"

#include <cassert>
#include <initializer_list>
#include <string_view>

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
	case TokenKind::UNKNOWN: return "UNKNOWN";

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
	case TokenKind::DOT: return "DOT";
	case TokenKind::UNDERSCORE: return "UNDERSCORE";
	case TokenKind::DOLLAR_SIGN: return "DOLLAR_SIGN";

	// Multi-character tokens
	case TokenKind::DOUBLE_EQUAL: return "DOUBLE_EQUAL";
	case TokenKind::BANG_EQUAL: return "BANG_EQUAL";
	case TokenKind::GREATER_EQUAL: return "GREATER_EQUAL";
	case TokenKind::LESS_EQUAL: return "LESS_EQUAL";
	case TokenKind::ARROW: return "ARROW";
	case TokenKind::RANGE_OP: return "RANGE_OP";

	// Literals
	case TokenKind::IDENTIFIER: return "IDENTIFIER";
	case TokenKind::STRING: return "STRING";
	case TokenKind::INTEGER: return "INTEGER";
	case TokenKind::FLOAT: return "FLOAT";

	// Keywords
	case TokenKind::AND: return "AND";
	case TokenKind::OR: return "OR";
	case TokenKind::IF: return "IF";
	case TokenKind::ELSE: return "ELSE";
	case TokenKind::TRUE: return "TRUE";
	case TokenKind::FALSE: return "FALSE";
	case TokenKind::WHILE: return "WHILE";
	case TokenKind::FOR: return "FOR";
	case TokenKind::RETURN: return "RETURN";
	case TokenKind::CONTINUE: return "CONTINUE";
	case TokenKind::BREAK: return "BREAK";
	case TokenKind::PASS: return "PASS";
	case TokenKind::LET: return "LET";
	case TokenKind::FUNC: return "FUNC";
	case TokenKind::END: return "END";

	case TokenKind::VOID_T: return "VOID_T";
	case TokenKind::INT_T: return "INT_T";
	case TokenKind::FLOAT_T: return "FLOAT_T";
	case TokenKind::BOOL_T: return "BOOL_T";
	case TokenKind::STRING_T: return "STRING_T";
	case TokenKind::SPRITE_T: return "SPRITE_T";
	
	case TokenKind::DIRECT_SPRITE: return "DIRECT_SPRITE";
	case TokenKind::DIRECT_USE: return "DIRECT_USE";
	case TokenKind::DIRECT_SELF: return "DIRECT_SELF";
	case TokenKind::DIRECT_UPDATE: return "DIRECT_UPDATE";
	case TokenKind::DIRECT_COLLIDE: return "DIRECT_COLLIDE";

	case TokenKind::COMMAND: return "COMMAND";
    }

    return "UNKNOWN_TOKEN";
}

TokenKind token_to_type(TokenKind kind) {
	switch (kind) {
	case TokenKind::STRING: return TokenKind::STRING_T;
	case TokenKind::INTEGER: return TokenKind::INT_T;
	case TokenKind::FLOAT: return TokenKind::FLOAT_T;
	case TokenKind::TRUE:
	case TokenKind::FALSE:
		return TokenKind::BOOL_T;
	default: 
		LOG_ERR("This TokenKind can not be interpret as a type");
		exit(1);
	}
}

TokenKind property_to_type(char prop) {
	switch (prop) {
	case PROP_Y_LEX:
	case PROP_X_LEX:
		return TokenKind::INT_T;
	default:
		LOG_ERR("Property does not exist.");
		exit(1);
	}
}

bool token_to_boolean(TokenKind kind) {
	assert(token_is_boolean(kind));

	switch (kind) {
	case TokenKind::TRUE: return true;
	case TokenKind::FALSE: return false;
	default: 
		LOG_ERR("This token can't be convert to boolean");
		exit(1);
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

bool token_is_logical_operator(TokenKind kind) {
	switch (kind) {
	case TokenKind::AND:
	case TokenKind::OR:
		return true;
	default:
		return false;
	}
}

bool token_is_comparison_operator(TokenKind kind) {
	switch (kind) {
	case TokenKind::DOUBLE_EQUAL:
	case TokenKind::BANG_EQUAL:
	case TokenKind::GREATER_EQUAL:
	case TokenKind::LESS_EQUAL:
	case TokenKind::GREATER:
	case TokenKind::LESS:
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


bool token_is_value_type(TokenKind kind) {
	switch (kind) {
	case TokenKind::INT_T:
	case TokenKind::FLOAT_T:
	case TokenKind::BOOL_T:
	case TokenKind::STRING_T:
		return true;
	default:
		return false;
	}
}

bool token_is_type(TokenKind kind) {
	if (token_is_value_type(kind)) {
		return true;
	}

	switch (kind) {
	case TokenKind::VOID_T:
	case TokenKind::SPRITE_T:
		return true;
	default:
		return false;
	}
}

bool token_is_boolean(TokenKind kind) {
	switch (kind) {
	case TokenKind::TRUE:
	case TokenKind::FALSE:
		return true;
	default:
		return false;
	}
}

bool token_is_property(const Token& token) {
	if (token.kind != TokenKind::IDENTIFIER || !token.lexeme) {
		return false;
	}

	const auto& lexeme = *token.lexeme;

	if (lexeme.size() > 1 || lexeme.size() == 0) {
		return false;
	}

	switch (lexeme.front()) {
	case PROP_X_LEX:
	case PROP_Y_LEX:
		return true;
	default:
		return false;
	}
}

} // namespace scr
