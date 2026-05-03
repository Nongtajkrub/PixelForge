#pragma once

#include "fscript_source_stream.hpp"
#include "fscript_diagnostic.hpp"
#include "fscript_location.hpp"
#include "fscript_token.hpp"

#include <cstddef>
#include <string_view>
#include <vector>

namespace scr {

class Lexer {
private:
	std::vector<Token> tokens;
	SourceStream<std::string, char> source;

	Location location;
	std::ostream& err_stream;

public:
	explicit Lexer(const std::string& source, std::ostream& err_stream) :
		source(SourceStream<std::string, char>(source)),
		location(1, 1),
		err_stream(err_stream)
	{ }

	bool lex();

	inline std::vector<Token>& get_token() {
		return this->tokens;
	}

private:
	inline void add_token(TokenKind type) {
		this->tokens.push_back(Token(type, this->location));
	}

	inline void add_token(TokenKind type, std::string_view lexeme) {
		this->tokens.push_back(Token(type, lexeme, this->location));
	}

	inline void emit(DiagnosticKind kind) {
		Diagnostic(kind, this->location).emit(this->err_stream);
	}
};

} // namespace scr
