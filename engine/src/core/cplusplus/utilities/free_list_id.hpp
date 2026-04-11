#pragma once

#include "incremental_id.hpp"

#include <algorithm>
#include <cassert>
#include <vector>

namespace core {

template <typename T>
requires IdType<T>
class FreeListIdGen {
private:
	T counter;
	std::vector<T> free_list;

public:
	FreeListIdGen(T start) :
		counter(start)
	{ }

	inline T generate() {
		if (free_list.empty()) {
			return counter++;
		} else {
			const auto id = free_list.back();
			free_list.pop_back();
			return id;
		}
	}

	inline void free(T id) {
		// Ensure ID was even generate.
		assert(id < counter); 
		// Ensure ID was not already free.
		assert(
			std::find(this->free_list.begin(), free_list.end(), id) 
				== this->free_list.end());

		this->free_list.push_back(id);
	}
};

} // namespace core
