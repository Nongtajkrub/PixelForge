#pragma once

#include "../global.hpp"

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
	T prev = 0;

public:
	IncrementalIdGen(T start) :
		prev(start)
	{ }

	T generate() {
		return this->prev++;
	}
};

} // namespace core
