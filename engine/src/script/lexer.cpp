#include "lexer.hpp"
#include "token.hpp"
#include <cctype>
#include <unordered_map>

namespace scr {

static const std::unordered_map<std::string, TokenType> keywords = {
	{"and", TokenType::AND},
	{"or", TokenType::OR},
	{"if", TokenType::IF},
	{"endif", TokenType::ENDIF},
	{"else", TokenType::ELSE},
	{"true", TokenType::TRUE},
	{"false", TokenType::FALSE},
	{"while", TokenType::WHILE},
	{"for", TokenType::FOR},
	{"return", TokenType::RETURN},
	{"pass", TokenType::PASS},
	{"var", TokenType::VAR},
	{"print", TokenType::PRINT},
};

void Lexer::lex() {
	char c = this->source.advance();
	while (!this->source.is_eof()) {
		switch (c) {
			case '(':
				add_token(TokenType::LEFT_BRACE);
				break;
			case ')':
				add_token(TokenType::RIGHT_BRACE);
				break;
			case ',':
				add_token(TokenType::COMMA);
				break;
			case '-':
				add_token(TokenType::MINUS);
				break;
			case '+':
				add_token(TokenType::PLUS);
				break;
			case '/':
				add_token(TokenType::SLASH);
				break;
			case '*':
				add_token(TokenType::STAR);
				break;
			case ';':
				add_token(TokenType::SEMICOLON);
				break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				add_token(
					TokenType::NUMBER,
					this->source.advance_until(
						[](auto c) -> bool { return !std::isdigit(c); }));
				break;
			case '=':
				add_token(
					(this->source.advance_if('=')) ?
						TokenType::DOUBLE_EQUAL : TokenType::EQUAL);
				break;
			case '!':
				add_token(
					this->source.advance_if('=') ?
						TokenType::BANG_EQUAL : TokenType::BANG);
				break;
			case '>':
				add_token(
					this->source.advance_if('=') ?
						TokenType::GREATER_EQUAL : TokenType::GREATER);
				break;
			case '<':
				add_token(
					this->source.advance_if('=') ?
						TokenType::LESS_EQUAL : TokenType::LESS);
			case '"':
				// Advance once to skip the first quote.
				this->source.advance();

				add_token(TokenType::STRING, this->source.advance_until('"'));

				// Advance again to skip the last quote.
				this->source.advance();
				break;
			case '\n':
			case '\r':
				this->location.line++;
				break;
			case ' ':
			case '\t':
				break;
			default:
				const std::string lexeme =
					this->source.advance_until(
						[](auto c) -> bool { return !std::isalpha(c) && c != '_'; });

				if (auto it = keywords.find(lexeme); it != keywords.end()) {
					add_token(it->second);
				} else {
					add_token(TokenType::IDENTIFIER, lexeme);
				}

				break;
		}

		c = this->source.advance();
	}
}

} // namespace scr
