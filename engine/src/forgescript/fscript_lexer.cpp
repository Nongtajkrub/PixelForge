#include "fscript_lexer.hpp"

#include "fscript_diagnostic.hpp"
#include "fscript_location.hpp"
#include "fscript_token.hpp"
#include "fscript_specs.h"

#include <optional>
#include <ostream>
#include <string_view>
#include <unordered_map>

namespace scr {

static const std::unordered_map<std::string, TokenKind> keywords = {
	{"and", TokenKind::AND},
	{"or", TokenKind::OR},
	{"if", TokenKind::IF},
	{"else", TokenKind::ELSE},
	{"true", TokenKind::TRUE},
	{"false", TokenKind::FALSE},
	{"while", TokenKind::WHILE},
	{"for", TokenKind::FOR},
	{"continue", TokenKind::CONTINUE},
	{"break", TokenKind::BREAK},
	{"return", TokenKind::RETURN},
	{"pass", TokenKind::PASS},
	{"let", TokenKind::LET},
	{"func", TokenKind::FUNC},
	{"loop", TokenKind::LOOP},
	{"update", TokenKind::UPDATE},
	{"interface", TokenKind::INTERFACE},
	{"extend", TokenKind::EXTEND},
	{"end", TokenKind::END},
	{"@sprite", TokenKind::DIRECT_SPRITE},
	{"@use", TokenKind::DIRECT_USE},
	{"@self", TokenKind::DIRECT_SELF},
	{CMD_SPAWN_LEX, TokenKind::COMMAND},
	{CMD_DESPAWN_LEX, TokenKind::COMMAND},
	{CMD_UP_LEX, TokenKind::COMMAND},
	{CMD_DOWN_LEX, TokenKind::COMMAND},
	{CMD_RIGHT_LEX, TokenKind::COMMAND},
	{CMD_LEFT_LEX, TokenKind::COMMAND},
	{CMD_GOTO_LEX, TokenKind::COMMAND},
	{CMD_SHOW_LEX, TokenKind::COMMAND},
	{CMD_WAIT_LEX, TokenKind::COMMAND},
	{CMD_COLLIDE_LEX, TokenKind::COMMAND},
};

static inline void add_token(
	TokenBuffer& buf, TokenKind kind, const Location& loc) {
	buf.push_back(Token(kind, loc));
}

static inline void add_token(
	TokenBuffer& buf,
	TokenKind kind, std::string_view lexeme, const Location& loc) {
	buf.push_back(Token(kind, lexeme, loc));
}

std::optional<TokenBuffer> lex(const std::string& src, std::ostream& err_stream) {
	auto srcstream = SourceStream<std::string, char>(src);
	auto loc = Location(1, 1);
	auto buf = TokenBuffer{};

	while (!srcstream.is_eof()) {
		char c = srcstream.advance();
		loc.col++;

		switch (c) {
		case '(':
			add_token(buf, TokenKind::LEFT_PAREN, loc);
			break;
		case ')':
			add_token(buf, TokenKind::RIGHT_PAREN, loc);
			break;
		case '{':
			add_token(buf, TokenKind::LEFT_BRACE, loc);
			break;
		case '}':
			add_token(buf, TokenKind::RIGHT_BRACE, loc);
			break;
		case '[':
			add_token(buf, TokenKind::LEFT_BRACKET, loc);
			break;
		case ']':
			add_token(buf, TokenKind::RIGHT_BRACKET, loc);
			break;
		case ',':
			add_token(buf, TokenKind::COMMA, loc);
			break;
		case '+':
			add_token(buf, TokenKind::PLUS, loc);
			break;
		case '/':
			add_token(buf, TokenKind::SLASH, loc);
			break;
		case '*':
			add_token(buf, TokenKind::STAR, loc);
			break;
		case ';':
			add_token(buf, TokenKind::SEMICOLON, loc);
			break;
		case '_':
			add_token(buf, TokenKind::UNDERSCORE, loc);
			break;
		case '.':
			add_token(buf, TokenKind::DOT, loc);
			break;
		case '$':
			add_token(buf, TokenKind::DOLLAR_SIGN, loc);
			break;
		case ':':
			add_token(
				buf,
				(srcstream.match(':')) ? TokenKind::RANGE_OP : TokenKind::COLON, 
				loc);
			break;
		case '=':
			add_token(
				buf,
				(srcstream.match('=')) ? 
					TokenKind::DOUBLE_EQUAL : TokenKind::EQUAL,
				loc);
			break;
		case '!':
			add_token(
				buf,
				srcstream.match('=') ? TokenKind::BANG_EQUAL : TokenKind::BANG,
				loc);
			break;
		case '>':
			add_token(
				buf,
				srcstream.match('=') ?
					TokenKind::GREATER_EQUAL : TokenKind::GREATER,
				loc);
			break;
		case '<':
			add_token(
				buf,
				srcstream.match('=') ? TokenKind::LESS_EQUAL : TokenKind::LESS,
				loc);
			break;
		case '"': {
			// Advance once to skip the first quote.
			srcstream.advance();

			const auto sub_info = srcstream.advance_until('"');

			if (srcstream.is_eof()) {
				Diagnostic(DiagnosticKind::UNTERMINATED_STRING).emit(err_stream);
				return std::nullopt;
			}

			add_token(
				buf,
				TokenKind::STRING_LIT,
				srcstream.data().substr(sub_info.begin, sub_info.size), loc);
			loc.col += sub_info.size;

			// Advance again to skip the last quote.
			srcstream.advance();
			break;
		}
		case '-':
			if (std::isdigit(srcstream.peek())) {
				break;
			}

			add_token(
				buf,
				(srcstream.match('>')) ? TokenKind::ARROW : TokenKind::MINUS, loc);
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
		case '9': {
			const bool neg = srcstream.prev() == '-';
			const auto sub_info =
				srcstream.advance_until(
					[](auto c) -> bool {
						return !std::isdigit(c) && c != '.'; 
					});
			const auto lexeme = 
				(neg) ? "-" : "" 
					+ srcstream.data().substr(sub_info.begin, sub_info.size);

			add_token(
				buf,
				(lexeme.contains('.')) ?
					TokenKind::FLOAT_LIT : TokenKind::INTEGER_LIT,
				lexeme, loc);

			loc.col += sub_info.size;
			break;
		}
		case '#':
			if (srcstream.peek() == '<') {
				srcstream.advance_until('>');
				srcstream.advance();
			} else {
				srcstream.advance_until(
					[](auto c) -> bool { return c == '\n' || c == '\r'; });
			}
			break;
		case '\n':
		case '\r':
			loc.row++;
			loc.col = 0;
			break;
		case ' ':
		case '\t':
			break;
		default: {
			const auto sub_info = 
				srcstream.advance_until(
					[](auto c) -> bool { 
						return !std::isalpha(c) 
							&& c != '_' && !std::isdigit(c); });
			const auto lexeme = 
				srcstream.data().substr(sub_info.begin, sub_info.size);
			loc.col += sub_info.size;

			// If the keyword is a command then keep it's lexeme as well.
			if (auto it = keywords.find(lexeme); it != keywords.end()) {
				const auto kind = it->second;

				if (kind == TokenKind::COMMAND) {
					add_token(buf, kind, lexeme, loc);
				} else {
					add_token(buf, kind, loc);
				}
			} else {
				add_token(buf, TokenKind::IDENTIFIER, lexeme, loc);
			}

			break;
		}
		}
	}

	return buf;
}

} // namespace scr
