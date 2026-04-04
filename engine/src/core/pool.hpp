#pragma once

#include <cstddef>
#include <utility>
#include <vector>

namespace core {

template <typename T>
class Pool {
private:
	std::vector<T> pool;

public:
	Pool() = default;

	inline size_t push(T value) {
		this->pool.push_back(value);
		return this->pool.size() - 1;
	}

	inline size_t push(T&& value) {
		this->pool.push_back(std::move(value));
		return this->pool.size() - 1;
	}

	template <typename... Args>
	inline T& emplace(Args&&... args) {
		pool.emplace_back(std::forward<Args>(args)...);
		return pool.back();
	}

	inline size_t size() const {
		return this->pool.size();
	}

	T& operator[](size_t index) {
		return this->pool[index];
	}

	const T& operator[](size_t index) const {
		return this->pool[index];
	}
};

} // namespace core;
