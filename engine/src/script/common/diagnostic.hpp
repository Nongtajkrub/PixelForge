#pragma once

#include "../../global.hpp"
#include "location.hpp"
#include "token.hpp"

#include <cstddef>
#include <ostream>
#include <string_view>

namespace scr {

enum class DiagnosticKind : u8 {
	UNEXPECTED_EOF,
	UNEXPECTED_TOKEN,

	EXPECTED_SEMICOLON,
	EXPECTED_COLON,
	EXPECTED_IDENTIFIER,
	EXPECTED_LEFT_PAREN,
	EXPECTED_RIGHT_PAREN,
	EXPECTED_ARROW,
	EXPECTED_COMMA,
	EXPECTED_COMMAND,
	EXPECTED_NUMBER,
	EXPECTED_SPRITE_DIRECT,

	UNKNOWN_IDENTIFIER,
};

DiagnosticKind resolve_diag_expect_kind(TokenKind kind);

struct Diagnostic {
	DiagnosticKind kind;
	Location location;

	explicit Diagnostic(DiagnosticKind kind, const Token& from) :
		kind(kind),
		location(from.location)
	{ } 

	explicit Diagnostic(DiagnosticKind kind, Location location) :
		kind(kind),
		location(location)
	{ } 

	std::string_view resolve_msg() const;
	void emit(std::ostream& stream) const;
};

} // namespace scr
