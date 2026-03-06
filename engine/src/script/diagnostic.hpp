#pragma once

#include "../global.hpp"
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
	EXPECTED_COMMA,
};

struct Diagnostic {
	DiagnosticKind kind;
	Location location;

	explicit Diagnostic(DiagnosticKind kind, const Token& from) :
		kind(kind),
		location(from.location)
	{ } 

	std::string_view resolve_msg() const;
	void emit(std::ostream& stream) const;
};

} // namespace scr
