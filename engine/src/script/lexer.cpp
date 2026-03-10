#include "lexer.hpp"
#include "token.hpp"
#include <cctype>
#include <unordered_map>

namespace scr {

static const std::unordered_map<std::string, TokenKind> keywords = {
	{"and", TokenKind::AND},
	{"or", TokenKind::OR},
	{"if", TokenKind::IF},
	{"endif", TokenKind::ENDIF},
	{"else", TokenKind::ELSE},
	{"true", TokenKind::TRUE},
	{"self", TokenKind::SELF},
	{"false", TokenKind::FALSE},
	{"while", TokenKind::WHILE},
	{"for", TokenKind::FOR},
	{"return", TokenKind::RETURN},
	{"pass", TokenKind::PASS},
	{"let", TokenKind::LET},
	{"func", TokenKind::FUNC},
	{"endfunc", TokenKind::ENDFUNC},
	{"@sprite", TokenKind::DIRECT_SPRITE},
	{"@import", TokenKind::DIRECT_IMPORT},
	{"UP", TokenKind::CMD_UP},
	{"DOWN", TokenKind::CMD_DOWN},
	{"RIGHT", TokenKind::CMD_RIGHT},
	{"LEFT", TokenKind::CMD_LEFT},
	{"GOTO", TokenKind::CMD_GOTO},
	{"SPAWN", TokenKind::CMD_SPAWN},
};

void Lexer::lex() {
	while (!this->source.is_eof()) {
		char c = this->source.advance();
		this->location.col++;

		switch (c) {
			case '(':
				add_token(TokenKind::LEFT_PAREN);
				break;
			case ')':
				add_token(TokenKind::RIGHT_PAREN);
				break;
			case '{':
				add_token(TokenKind::LEFT_BRACE);
				break;
			case '}':
				add_token(TokenKind::RIGHT_BRACE);
				break;
			case '[':
				add_token(TokenKind::LEFT_BRACKET);
				break;
			case ']':
				add_token(TokenKind::RIGHT_BRACKET);
				break;
			case ',':
				add_token(TokenKind::COMMA);
				break;
			case '-':
				add_token(TokenKind::MINUS);
				break;
			case '+':
				add_token(TokenKind::PLUS);
				break;
			case '/':
				add_token(TokenKind::SLASH);
				break;
			case '*':
				add_token(TokenKind::STAR);
				break;
			case ';':
				add_token(TokenKind::SEMICOLON);
				break;
			case ':':
				add_token(TokenKind::COLON);
				break;
			case '=':
				add_token(
					(this->source.match('=')) ?
						TokenKind::DOUBLE_EQUAL : TokenKind::EQUAL);
				break;
			case '!':
				add_token(
					this->source.match('=') ?
						TokenKind::BANG_EQUAL : TokenKind::BANG);
				break;
			case '>':
				add_token(
					this->source.match('=') ?
						TokenKind::GREATER_EQUAL : TokenKind::GREATER);
				break;
			case '<':
				add_token(
					this->source.match('=') ?
						TokenKind::LESS_EQUAL : TokenKind::LESS);
				break;
			case '"': {
				// Advance once to skip the first quote.
				this->source.advance();

				const auto sub_info = this->source.advance_until('"');
				add_token(
					TokenKind::STRING,
					this->source.data().substr(sub_info.begin, sub_info.size));
				this->location.col += sub_info.size;

				// Advance again to skip the last quote.
				this->source.advance();
				break;
			}
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9': {
				const auto sub_info =
					this->source.advance_until(
						[](auto c) -> bool {
							return !std::isdigit(c) && c != '.'; });
				add_token(
					TokenKind::NUMBER,
					this->source.data().substr(sub_info.begin, sub_info.size));
				this->location.col += sub_info.size;

				break;
			}
			case '#':
				if (this->source.peek() == '<') {
					this->source.advance_until('>');
					this->source.advance();
				} else {
					this->source.advance_until(
						[](auto c) -> bool { return c == '\n' || c == '\r'; });
				}
				break;
			case '\n':
			case '\r':
				this->location.row++;
				this->location.col = 0;
				break;
			case ' ':
			case '\t':
				break;
			default: {
				const auto sub_info = 
					this->source.advance_until(
						[](auto c) -> bool { return !std::isalpha(c) && c != '_'; });
				const std::string lexeme =
						this->source.data().substr(sub_info.begin, sub_info.size);
				this->location.col += sub_info.size;

				if (auto it = keywords.find(lexeme); it != keywords.end()) {
					add_token(it->second);
				} else {
					add_token(TokenKind::IDENTIFIER, lexeme);
				}

				break;
			}
		}
	}
}

} // namespace scr
