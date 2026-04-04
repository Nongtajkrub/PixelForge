#pragma once

#include <cassert>
#include <cstddef>
#include <vector>

namespace core {

// Wrapper around std::vector.
template <typename T>
class CursorStack {
private:
	std::vector<T> stack = {};
	size_t cursor = 0;

public:
	CursorStack() = default;

	// Emplace back if no element already exist.
	template <typename ...Args>
	std::pair<T&, bool> try_emplace_back(Args&&... args) {
		bool inserted = false;
		if (cursor == this->stack.size()) {
			this->stack.emplace_back(std::forward<Args>(args)...);
			inserted = true;
		}
		cursor++;
		return {this->stack[cursor - 1], inserted};
	}

	void rewind() {
		assert(cursor != 0);
		cursor--;
	}

	inline T& top() {
		return this->stack[cursor - 1];
	}

	inline T& bottom() {
		return this->stack[0];
	}

	inline size_t size() const {
		return this->stack.size();
	}

	inline auto begin() {
		return this->stack.begin();
	}

	inline auto end() {
		return this->stack.end();
	}

	inline auto rbegin() {
		return this->stack.rbegin();
	}

	inline auto rend() {
		return this->stack.rend();
	}
};

} // namespace core
