#pragma once

#include "diagnostic.hpp"
#include "token.hpp"

#include <array>
#include <functional>
#include <optional>
#include <ostream>
#include <type_traits>

namespace scr {

// Simple pattern matching system using C++ type system.
// S: Stream type (TokenStream, MutTokenStream).
template <typename S, TokenKind... Expect>
requires (std::is_same_v<S, TokenStream> || std::is_same_v<S, MutTokenStream>)
struct Pattern {
	using OstreamRef = std::reference_wrapper<std::ostream>;

	static constexpr std::array<TokenKind, sizeof...(Expect)> expects{Expect...};

	// Match pattern and return the result, can also accept an ASTNodeBuffer to 
	// add the match tokens to the buffer as node.
	static bool match(
		S& stream,
		std::optional<OstreamRef> err_stream = std::nullopt) {
		for (const auto expect : expects) {
			if (stream.is_eof()) {
				if (err_stream) {
					Diagnostic(DiagnosticKind::UNEXPECTED_EOF, stream.prev())
						.emit(*err_stream);
				}
				return false;
			}

			if (expect == stream.peek().kind) {
				stream.advance();
			} else {
				if (err_stream) {
					Diagnostic(resolve_diag_expect_kind(expect), stream.prev())
						.emit(*err_stream);
				}
				return false;
			}
		}
		return true;
	}

	static bool match_peek(
		S& stream,
		std::optional<OstreamRef> err_stream = std::nullopt) {
		if (!match(stream, err_stream)) {
			return false;
		}

		stream.revert(sizeof...(Expect));
		return true;
	}
};

} // namespace scr
