#pragma once

#include "../types.hpp"

#include <cassert>
#include <concepts>

namespace core {

template <typename T>
concept IdType =
	std::same_as<T, u64> 
		|| std::same_as<T, u32> || std::same_as<T, u16> || std::same_as<T, u8>;

template <typename T>
requires IdType<T>
class IncrementalIdGen {
private:
	T start;
	T prev;

public:
	IncrementalIdGen(T start) :
		start(start), prev(start)
	{ }

	inline T generate() {
		return this->prev++;
	}

	inline void revert() {
		if constexpr (std::same_as<T, u8> 
			|| std::same_as<T, u16> 
			|| std::same_as<T, u32> || std::same_as<T, u64>) {
			assert(this->prev != 0);
		}

		this->prev--;
	}

	inline void reset() {
		this->prev = this->start;
	}
};

} // namespace core
