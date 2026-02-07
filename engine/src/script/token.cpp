#include "token.hpp"

#include <string_view>
#include <unordered_map>

namespace scr {

const std::string_view Token::type_as_str() const {
	static const std::unordered_map<TokenType, const char*> converter = {
		// Single character tokens
		{TokenType::LEFT_BRACE,    "LEFT_BRACE"},
		{TokenType::RIGHT_BRACE,   "RIGHT_BRACE"},
		{TokenType::COMMA,         "COMMA"},
		{TokenType::MINUS,         "MINUS"},
		{TokenType::PLUS,          "PLUS"},
		{TokenType::SLASH,         "SLASH"},
		{TokenType::STAR,          "STAR"},
		{TokenType::EQUAL,         "EQUAL"},
		{TokenType::SEMICOLON,     "SEMICOLON"},
		{TokenType::BANG,          "BANG"},
		{TokenType::GREATER,       "GREATER"},
		{TokenType::LESS,          "LESS"},

		// Multi-character tokens
		{TokenType::DOUBLE_EQUAL,  "DOUBLE_EQUAL"},
		{TokenType::BANG_EQUAL,    "BANG_EQUAL"},
		{TokenType::GREATER_EQUAL, "GREATER_EQUAL"},
		{TokenType::LESS_EQUAL,    "LESS_EQUAL"},

		// Literals
		{TokenType::IDENTIFIER,    "IDENTIFIER"},
		{TokenType::STRING,        "STRING"},
		{TokenType::NUMBER,        "NUMBER"},

		// Keywords
		{TokenType::AND,           "AND"},
		{TokenType::OR,            "OR"},
		{TokenType::IF,            "IF"},
		{TokenType::ENDIF,         "ENDIF"},
		{TokenType::ELSE,          "ELSE"},
		{TokenType::TRUE,          "TRUE"},
		{TokenType::FALSE,         "FALSE"},
		{TokenType::WHILE,         "WHILE"},
		{TokenType::FOR,           "FOR"},
		{TokenType::RETURN,        "RETURN"},
		{TokenType::PASS,          "PASS"},
		{TokenType::VAR,           "VAR"},
	};

	return converter.at(this->token);
}

} // namespace scr
