#pragma once

#include "../global.hpp"
#include "token.hpp"
#include "source_stream.hpp"

#include <cstddef>
#include <span>
#include <string_view>
#include <vector>

namespace scr {

class Lexer {
private:
	std::vector<Token> tokens;
	SourceStream<std::string, char> source;

	struct {
		u32 line;
	} location;

public:
	explicit Lexer(const std::string& source) :
		source(SourceStream<std::string, char>(source)),
		location{ .line = 0 }
	{ 
		this->lex();
	}

	inline const std::span<Token> get_token() {
		return this->tokens;
	}

private:
	void lex();

	inline void add_token(TokenType type) {
		this->tokens.push_back(Token(type, this->location.line));
	}

	inline void add_token(TokenType type, std::string_view lexeme) {
		this->tokens.push_back(Token(type, lexeme, this->location.line));
	}
};

} // namespace scr
