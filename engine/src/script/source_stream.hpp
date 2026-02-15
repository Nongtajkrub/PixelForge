#pragma once

#include <cstddef>
#include <functional>
#include <ranges>

namespace scr {

// T: Type of source to create a SourceStream from, must be a range.
// U: Type of element containe in source, if `std::string` the element is `char`.
// V: Type of an alternative `U`, usually a `&U` to avoid unnecessary copies. (Optional)
template <typename T, typename U, typename V = U>
requires std::ranges::random_access_range<T> 
class SourceStream {
public:
	struct SubstreamInfo {
		size_t begin;
		size_t size;
	};

private:
	const T soruce;

	struct {
		size_t pos;
	} location;

public:
	explicit SourceStream(T soruce) :
		soruce(soruce),
		location{ .pos = 0  }
	{ }

	// Consume the current charactor and move to the next.
	inline V advance() {
		V v = this->soruce[this->location.pos];
		this->location.pos++;

		return v;
	}

	inline V prev() {
		return this->soruce[this->location.pos - 1];
	}
	
	// Consume and move only if the current charactor is `c`.
	// Return whether an advance occur or not.
	inline bool match(U v) {
		if (peek() == v) {
			advance();
			return true;
		} else {
			return false;
		}
	}

	inline bool match(std::function<bool(V)> predicate) {
		if (predicate(peek())) {
			advance();
			return true;
		} else {
			return false;
		}
	}

	SubstreamInfo advance_until(U v) {
		const size_t begin = this->location.pos - 1;
		while (peek() != v && !is_eof()) { advance(); }
		return (SubstreamInfo) {
			.begin = begin,
			.size = this->location.pos - begin,
		};
	}

	SubstreamInfo advance_until(std::function<bool(V)> predicate) {
		const size_t begin = this->location.pos - 1;
		while (!predicate(peek()) && !is_eof()) { advance(); }
		return (SubstreamInfo) {
			.begin = begin,
			.size = this->location.pos - begin,
		};
	}

	// Peek at the current charactor without consuming.
	inline V peek() const {
		return this->soruce[this->location.pos];
	}

	inline size_t get_pos() const {
		return this->location.pos;
	}

	inline bool is_eof() const {
		return this->location.pos == this->soruce.size();
	}

	inline const T& data() const {
		return this->soruce;
	}
};

} // namespace scr
