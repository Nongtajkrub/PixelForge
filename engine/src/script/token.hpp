#pragma once

#include "../global.hpp"
#include "location.hpp"

#include <string>
#include <optional>
#include <string_view>

namespace scr {

enum class TokenKind : u8 {
	// Single charactor tokens
	LEFT_BRACE,
	RIGHT_BRACE,
	LEFT_BRACKET,
	RIGHT_BRACKET,
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
	VAR,
	PRINT,
	MATH,
};

struct Token {
	TokenKind kind;
	std::optional<std::string> lexeme;

	Location location;

	Token(TokenKind kind, Location location) :
		kind(kind),
		lexeme(std::nullopt),
		location(location)
	{ }
	Token(TokenKind kind, const std::string_view lexeme, Location location) :
		kind(kind),
		lexeme(lexeme),
		location(location)
	{ }

	const std::string_view kind_as_str() const;
	bool is_arithmetic_operator() const;
	
};

} // namespace scr
