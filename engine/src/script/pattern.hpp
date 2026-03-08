#pragma once

#include "diagnostic.hpp"
#include "token.hpp"
#include "source_stream.hpp"

#include <array>

namespace scr {

template <TokenKind... Expect>
struct Pattern {
	static constexpr std::array<TokenKind, sizeof...(Expect)> expects{Expect...};

	// Match pattern and return the result, can also accept an ASTNodeBuffer to 
	// add the match tokens to the buffer as node.
	static bool match(
		SourceStream<const std::span<Token>, Token, const Token&>& stream,
		std::ostream& emit_to) {
		for (const auto expect : expects) {
			if (stream.is_eof()) {
				Diagnostic(DiagnosticKind::UNEXPECTED_EOF, stream.prev())
					.emit(emit_to);
				return false;
			}

			if (expect == stream.peek().kind) {
				stream.advance();
			} else {
				Diagnostic(resolve_diag_expect_kind(expect), stream.peek())
					.emit(emit_to);
				return false;
			}
		}
		return true;
	}
};

} // namespace scr
