#pragma once

#include "../global.hpp"

#include <optional>
#include <string_view>

namespace scr {

enum class TokenType : u8 {
	// Single charactor tokens
	LEFT_BRACE,
	RIGHT_BRACE,
	COMMA,
	MINUS,
	PLUS,
	SLASH,
	STAR,
	EQUAL,
	SEMICOLON,
	BANG,
	GREATER,
	LESS,

	// Multiples charactor token
	DOUBLE_EQUAL,
	BANG_EQUAL,
	GREATER_EQUAL,
	LESS_EQUAL,

	// Literals
	IDENTIFIER,
	STRING,
	NUMBER,

	// Keywords
	AND,
	OR,
	IF,
	ENDIF,
	ELSE,
	TRUE,
	FALSE,
	WHILE,
	FOR,
	RETURN,
	PASS,
	VAR
};

struct Token {
	TokenType token;
	std::optional<std::string> lexeme;

	struct {
		u32 line;
	} location;

	Token(TokenType token, u32 line) :
		token(token),
		lexeme(std::nullopt),
		location{ .line = line }
	{ }
	Token(TokenType token, const std::string_view lexeme, u32 line) :
		token(token),
		lexeme(lexeme),
		location{ .line = line }
	{ }

	const std::string_view type_as_str() const;
};

} // namespace scr
