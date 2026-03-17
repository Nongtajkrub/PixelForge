#pragma once

#include <cstddef>
#include <utility>
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

	template <typename ...Args>
	inline void emplace_back(Args&& ...args) {
		this->stack.emplace_back(std::forward(args)...);
	}

	inline T pop() {
		T v = this->stack.back();
		this->stack.pop_back();
		return v;
	}

	inline T& top() {
		return this->stack.back();
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
