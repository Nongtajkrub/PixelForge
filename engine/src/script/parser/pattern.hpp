#pragma once

#include "../common/diagnostic.hpp"
#include "../common/token.hpp"
#include "../common/source_stream.hpp"

#include <array>
#include <functional>
#include <optional>
#include <ostream>

namespace scr {

template <TokenKind... Expect>
struct Pattern {
	using OstreamRef = std::reference_wrapper<std::ostream>;

	using TokensStream =
		SourceStream<const std::span<Token>, Token, const Token&>;

	static constexpr std::array<TokenKind, sizeof...(Expect)> expects{Expect...};

	// Match pattern and return the result, can also accept an ASTNodeBuffer to 
	// add the match tokens to the buffer as node.
	static bool match(
		TokensStream& stream,
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
		TokensStream& stream,
		std::optional<OstreamRef> err_stream = std::nullopt) {
		if (!match(stream, err_stream)) {
			return false;
		}

		stream.revert(sizeof...(Expect));
		return true;
	}
};

} // namespace scr
