#pragma once

#include <vector>

namespace core {

// Wrapper around std::vector.
template <typename T>
class IterableStack {
private:
	std::vector<T> stack = {};

public:
	inline void push(const T& v) {
		this->stack.push_back(v);
	}

	inline T pop() {
		T v = this->stack.back();
		this->stack.pop_back();
		return v;
	}

	inline const T& top() const {
		return this->stack.back();
	}

	inline auto bottom() {
		return this->stack[0];
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
