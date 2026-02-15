#include "token.hpp"

#include <string_view>
#include <unordered_map>

namespace scr {

const std::string_view Token::type_as_str() const {
	static const std::unordered_map<TokenKind, const char*> converter = {
		// Single character tokens
		{TokenKind::LEFT_BRACE,    "LEFT_BRACE"},
		{TokenKind::RIGHT_BRACE,   "RIGHT_BRACE"},
		{TokenKind::LEFT_BRACKET,  "LEFT_BRACKET"},
		{TokenKind::RIGHT_BRACKET, "RIGHT_BRACKET"},
		{TokenKind::COMMA,         "COMMA"},
		{TokenKind::MINUS,         "MINUS"},
		{TokenKind::PLUS,          "PLUS"},
		{TokenKind::SLASH,         "SLASH"},
		{TokenKind::STAR,          "STAR"},
		{TokenKind::EQUAL,         "EQUAL"},
		{TokenKind::SEMICOLON,     "SEMICOLON"},
		{TokenKind::BANG,          "BANG"},
		{TokenKind::GREATER,       "GREATER"},
		{TokenKind::LESS,          "LESS"},

		// Multi-character tokens
		{TokenKind::DOUBLE_EQUAL,  "DOUBLE_EQUAL"},
		{TokenKind::BANG_EQUAL,    "BANG_EQUAL"},
		{TokenKind::GREATER_EQUAL, "GREATER_EQUAL"},
		{TokenKind::LESS_EQUAL,    "LESS_EQUAL"},

		// Literals
		{TokenKind::IDENTIFIER,    "IDENTIFIER"},
		{TokenKind::STRING,        "STRING"},
		{TokenKind::NUMBER,        "NUMBER"},

		// Keywords
		{TokenKind::AND,           "AND"},
		{TokenKind::OR,            "OR"},
		{TokenKind::IF,            "IF"},
		{TokenKind::ENDIF,         "ENDIF"},
		{TokenKind::ELSE,          "ELSE"},
		{TokenKind::TRUE,          "TRUE"},
		{TokenKind::FALSE,         "FALSE"},
		{TokenKind::WHILE,         "WHILE"},
		{TokenKind::FOR,           "FOR"},
		{TokenKind::RETURN,        "RETURN"},
		{TokenKind::PASS,          "PASS"},
		{TokenKind::VAR,           "VAR"},
		{TokenKind::PRINT,         "PRINT"},
		{TokenKind::MATH,          "MATH"},
	};

	return converter.at(this->kind);
}

} // namespace scr
