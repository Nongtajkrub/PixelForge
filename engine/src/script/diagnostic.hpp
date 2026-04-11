#pragma once

#include "../core/cplusplus/types.hpp"
#include "location.hpp"
#include "token.hpp"

#include <cstddef>
#include <optional>
#include <ostream>
#include <string_view>

namespace scr {

enum class DiagnosticKind : u8 {
	FAIL_OPEN_SOURCE,

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
	EXPECTED_INTEGER,
	EXPECTED_FLOAT,
	EXPECTED_BOOL,
	EXPECTED_STRING,
	EXPECTED_SPRITE,
	EXPECTED_SPRITE_DIRECT,
	EXPECTED_TYPE,
	EXPECTED_VALUE_TYPE,
	EXPECTED_PROPERTY,
	EXPECTED_FUNCTION,
	EXPECTED_RANGE_OP,
	EXPECTED_RANGE_EXPR,

	UNKNOWN_IDENTIFIER,

	UNTERMINATED_STRING,
	TYPE_ERROR,
	TO_MANY_ARGUMENTS,
	JUMP_NOT_ALLOW,
	FUNC_DECL_NOT_ALLOW,
	RETURN_NOT_ALLOW,
	VARIABLE_REDECLARATION,
};

DiagnosticKind resolve_diag_expect_kind(TokenKind kind);

struct Diagnostic {
	DiagnosticKind kind;
	std::optional<Location> location;

	explicit Diagnostic(DiagnosticKind kind, const Token& from) :
		kind(kind),
		location(from.location)
	{ } 
	explicit Diagnostic(DiagnosticKind kind, Location location) :
		kind(kind),
		location(location)
	{ } 
	explicit Diagnostic(DiagnosticKind kind) :
		kind(kind),
		location(std::nullopt)
	{ } 

	std::string_view resolve_msg() const;
	void emit(std::ostream& stream) const;
};

} // namespace scr
