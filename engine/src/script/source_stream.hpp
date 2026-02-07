#pragma once

#include <cstddef>
#include <utility>
#include <functional>
#include <ranges>

namespace scr {

// T: Type of source to create a SourceStream from, must be a range.
// U: Type of element containe in source, if `std::string` the element is `char`.
template <typename T, typename U>
requires std::ranges::random_access_range<T> 
class SourceStream {
private:
	const T soruce;

	struct {
		size_t pos;
	} location;

public:
	explicit SourceStream(const T& soruce) :
		soruce(std::move(soruce)),
		location{ .pos = 0  }
	{ }

	// Consume the current charactor and move to the next.
	U advance() {
		if (is_eof()) return '\0';

		char c = this->soruce[this->location.pos];
		this->location.pos++;

		return c;
	}
	
	// Consume and move only if the current charactor is `c`.
	// Return whether an advance occur or not.
	bool advance_if(U v) {
		if (peek() == v) {
			advance();
			return true;
		} else {
			return false;
		}
	}

	bool advance_if(std::function<bool(U)> predicate) {
		if (predicate(peek())) {
			advance();
			return true;
		} else {
			return false;
		}
	}

	T advance_until(U v) {
		const size_t begin = this->location.pos - 1;
		while (peek() != v && !is_eof()) { advance(); }

		return this->soruce 
			| std::views::drop(begin) 
			| std::views::take(this->location.pos - begin)
			| std::ranges::to<T>();
	}

	T advance_until(std::function<bool(U)> predicate) {
		const size_t begin = this->location.pos - 1;
		while (!predicate(peek()) && !is_eof()) { advance(); }

		return this->soruce 
			| std::views::drop(begin) 
			| std::views::take(this->location.pos - begin)
			| std::ranges::to<T>();
	}

	// Peek at the current charactor without consuming.
	inline U peek() const {
		return this->soruce[this->location.pos];
	}

	inline size_t get_pos() const {
		return this->location.pos;
	}

	inline bool is_eof() const {
		return this->location.pos == this->soruce.size();
	}
};

} // namespace scr
