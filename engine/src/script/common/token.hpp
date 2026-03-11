#pragma once

#include "../../global.hpp"
#include "location.hpp"

#include <string>
#include <optional>

namespace scr {

enum class TokenKind : u8 {
	// Single charactor tokens
	LEFT_BRACE,
	RIGHT_BRACE,
	LEFT_BRACKET,
	RIGHT_BRACKET,
	LEFT_PAREN,
	RIGHT_PAREN,
	COMMA,
	MINUS,
	PLUS,
	SLASH,
	STAR,
	EQUAL,
	SEMICOLON,
	COLON,
	BANG,
	GREATER,
	LESS,

	// Multiples charactor token
	DOUBLE_EQUAL,
	BANG_EQUAL,
	GREATER_EQUAL,
	LESS_EQUAL,
	ARROW,

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
	SELF,
	WHILE,
	FOR,
	RETURN,
	PASS,
	LET,
	FUNC,
	ENDFUNC,

	DIRECT_SPRITE,
	DIRECT_USE,

	COMMAND,
};

// Differents lexeme for each commands.
constexpr const char* CMD_SPAWN_LEX = "SPAWN";
constexpr const char* CMD_DESPAWN_LEX = "DESPAWN";
constexpr const char* CMD_UP_LEX = "UP";
constexpr const char* CMD_DOWN_LEX = "DOWN";
constexpr const char* CMD_RIGHT_LEX = "RIGHT";
constexpr const char* CMD_LEFT_LEX = "LEFT";
constexpr const char* CMD_GOTO_LEX = "GOTO";
constexpr const char* CMD_SHOW_LEX = "SHOW";
constexpr const char* CMD_UPDATE_LEX = "UPDATE";
constexpr const char* CMD_COLLIDE_LEX = "COLLIDE";

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

	const char* kind_as_str() const;
	bool is_arithmetic_operator() const;
	bool is_directive() const;
};

}
