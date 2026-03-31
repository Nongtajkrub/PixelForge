#include "token.hpp"
#include "lexer.hpp"

#include <cctype>
#include <unordered_map>

namespace scr {

static const std::unordered_map<std::string, TokenKind> keywords = {
	{"and", TokenKind::AND},
	{"or", TokenKind::OR},
	{"if", TokenKind::IF},
	{"else", TokenKind::ELSE},
	{"true", TokenKind::TRUE},
	{"self", TokenKind::SELF},
	{"false", TokenKind::FALSE},
	{"while", TokenKind::WHILE},
	{"for", TokenKind::FOR},
	{"continue", TokenKind::CONTINUE},
	{"break", TokenKind::BREAK},
	{"return", TokenKind::RETURN},
	{"pass", TokenKind::PASS},
	{"let", TokenKind::LET},
	{"func", TokenKind::FUNC},
	{"end", TokenKind::END},
	{"@sprite", TokenKind::DIRECT_SPRITE},
	{"@use", TokenKind::DIRECT_USE},
	{"@update", TokenKind::DIRECT_UPDATE},
	{"@collide", TokenKind::DIRECT_COLLIDE},
	{VOID_T_LEX, TokenKind::VOID_T},
	{INT_T_LEX, TokenKind::INT_T},
	{FLOAT_T_LEX, TokenKind::FLOAT_T},
	{BOOL_T_LEX, TokenKind::BOOL_T},
	{STRING_T_LEX, TokenKind::STRING_T},
	{SPRITE_T_LEX, TokenKind::SPRITE_T},
	{CMD_SPAWN_LEX, TokenKind::COMMAND},
	{CMD_DESPAWN_LEX, TokenKind::COMMAND},
	{CMD_UP_LEX, TokenKind::COMMAND},
	{CMD_DOWN_LEX, TokenKind::COMMAND},
	{CMD_RIGHT_LEX, TokenKind::COMMAND},
	{CMD_LEFT_LEX, TokenKind::COMMAND},
	{CMD_GOTO_LEX, TokenKind::COMMAND},
	{CMD_SHOW_LEX, TokenKind::COMMAND},
	{CMD_UPDATE_LEX, TokenKind::COMMAND},
	{CMD_COLLIDE_LEX, TokenKind::COMMAND},
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
		case '_':
			add_token(TokenKind::UNDERSCORE);
			break;
		case '.':
			add_token(TokenKind::DOT);
			break;
		case ':':
			add_token(
				(this->source.match(':')) ?
					TokenKind::RANGE_OP : TokenKind::COLON);
			break;
		case '-':
			add_token(
				(this->source.match('>')) ?
					TokenKind::ARROW : TokenKind::MINUS);
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
			const auto lexeme = 
				this->source.data().substr(sub_info.begin, sub_info.size);
			add_token(
				(lexeme.contains('.')) ?
					TokenKind::FLOAT : TokenKind::INTEGER, lexeme);

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
					[](auto c) -> bool { 
						return !std::isalpha(c) 
							&& c != '_' && !std::isdigit(c); });
			const std::string lexeme =
			this->source.data().substr(sub_info.begin, sub_info.size);
			this->location.col += sub_info.size;

			// If the keyword is a command then keep it's lexeme as well.
			if (auto it = keywords.find(lexeme); it != keywords.end()) {
				const auto kind = it->second;

				if (kind == TokenKind::COMMAND) {
					add_token(kind, lexeme);
				} else {
					add_token(kind);
				}
			} else {
				add_token(TokenKind::IDENTIFIER, lexeme);
			}

			break;
		}
		}
	}
}

} // namespace scr
