#pragma once

#include "../global.hpp"
#include "source_stream.hpp"
#include "location.hpp"

#include <initializer_list>
#include <ostream>
#include <string>
#include <optional>
#include <span>
#include <string_view>

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
	DOT,
	UNDERSCORE,

	// Multiples charactor token
	DOUBLE_EQUAL,
	BANG_EQUAL,
	GREATER_EQUAL,
	LESS_EQUAL,
	ARROW,
	RANGE_OP,

	// Literals
	IDENTIFIER,
	STRING,
	INTEGER,
	FLOAT,

	// Keywords
	AND,
	OR,
	IF,
	ELSE,
	TRUE,
	FALSE,
	SELF,
	WHILE,
	FOR,
	CONTINUE,
	BREAK,
	RETURN,
	PASS,
	LET,
	FUNC,
	END,

	VOID_T,
	INT_T,
	FLOAT_T,
	BOOL_T,
	STRING_T,
	SPRITE_T,

	DIRECT_SPRITE,
	DIRECT_USE,
	DIRECT_UPDATE,
	DIRECT_COLLIDE,

	COMMAND,
};

// Differents lexeme for each commands.
constexpr const char* CMD_UP_LEX = "UP";
constexpr const char* CMD_DOWN_LEX = "DOWN";
constexpr const char* CMD_RIGHT_LEX = "RIGHT";
constexpr const char* CMD_LEFT_LEX = "LEFT";
constexpr const char* CMD_GOTO_LEX = "GOTO";
constexpr const char* CMD_SPAWN_LEX = "SPAWN";
constexpr const char* CMD_DESPAWN_LEX = "DESPAWN";
constexpr const char* CMD_SHOW_LEX = "SHOW";
constexpr const char* CMD_UPDATE_LEX = "UPDATE";
constexpr const char* CMD_COLLIDE_LEX = "COLLIDE";

// Built in types lexemes.
constexpr const char* VOID_T_LEX = "void";
constexpr const char* INT_T_LEX = "int";
constexpr const char* FLOAT_T_LEX = "float";
constexpr const char* BOOL_T_LEX = "bool";
constexpr const char* STRING_T_LEX = "str";
constexpr const char* SPRITE_T_LEX = "Sprite";

// Builtin property lexemes.
constexpr const char PROP_X_LEX = 'x';
constexpr const char PROP_Y_LEX = 'y';

struct Token {
	TokenKind kind;

	// Set by the preprocessor to indicate that the parser should skip this token.
	bool skip = false;

	std::optional<std::string> lexeme;

	Location location;

	explicit Token() = default;
	explicit Token(TokenKind kind, Location location) :
		kind(kind),
		lexeme(std::nullopt),
		location(location)
	{ }
	explicit Token(
		TokenKind kind, const std::string_view lexeme, Location location) :
		kind(kind),
		lexeme(lexeme),
		location(location)
	{ }
};

// A classic none mutable token stream, extending the SourceStream class.
class TokenStream : 
	public SourceStream<const std::span<Token>, Token, const Token&> {
private:
	std::ostream& err_stream;

public:
	TokenStream(const std::span<Token> tokens, std::ostream& err_stream) :
		SourceStream(tokens), err_stream(err_stream)
	{ }

	bool ensure_not_eof();
	bool expect(TokenKind kind);
	bool expect_peek(TokenKind kind);

	inline bool match_kind(TokenKind kind) {
		if (!ensure_not_eof()) {
			return false;
		}
		return match([kind](auto token) -> bool { return token.kind == kind; });
	}
};

// A mutable token stream enabling replace and insert, extending the SourceStream class.
class MutTokenStream :
	public SourceStream<std::vector<Token>&, Token, Token&> {
public:
	MutTokenStream(std::vector<Token>& tokens) :
		SourceStream(tokens)
	{ }

	void replace_and_insert(std::initializer_list<Token> v);

	// Consume the token and mark it as skip for the parser.
	inline const Token& skip() {
		Token& token = advance();
		token.skip = true;
		return token;
	}
};

// Convert TokenKind to string (IDENTIFIER -> "IDENTIFIER").
const char* token_kind_as_str(TokenKind kind);

// Convert TokenKind to types kind (INTEGER -> INT_T, STRING -> STRING_T).
// Will panic if match is not possible.
TokenKind token_to_type(TokenKind kind);
TokenKind property_to_type(char prop);
bool token_to_boolean(TokenKind kind);

bool token_is_arithmetic_operator(TokenKind kind);
bool token_is_logical_operator(TokenKind kind);
bool token_is_comparison_operator(TokenKind kind);
bool token_is_directive(TokenKind kind);
bool token_is_value_type(TokenKind kind);
bool token_is_type(TokenKind kind);
bool token_is_boolean(TokenKind kind);
bool token_is_property(const Token& token);

}
