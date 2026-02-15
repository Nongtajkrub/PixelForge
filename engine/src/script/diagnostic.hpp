#pragma once

#include "../global.hpp"
#include "location.hpp"
#include "token.hpp"

#include <cstddef>
#include <ostream>
#include <string_view>

namespace scr {

enum class DiagnosticKind : u8 {
    // Syntax Errors
    UNEXPECTED_TOKEN,
    UNEXPECTED_END_OF_FILE,
    EXPECTED_TOKEN,
    EXPECTED_IDENTIFIER,
    EXPECTED_EXPRESSION,
    EXPECTED_TYPE,
    EXPECTED_SEMICOLON,
    EXPECTED_RIGHT_PAREN,
    EXPECTED_RIGHT_BRACE,
    INVALID_ASSIGNMENT_TARGET,
    TOO_MANY_ARGUMENTS,

    // Name Resolution
    UNKNOWN_IDENTIFIER,
    REDECLARED_IDENTIFIER,

    // Type Checking 
    TYPE_MISMATCH,
    INVALID_BINARY_OPERANDS,
    CANNOT_ASSIGN_TO_CONST,

    // Warnings
    UNUSED_VARIABLE,
    UNREACHABLE_CODE,
    SHADOWED_VARIABLE
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
