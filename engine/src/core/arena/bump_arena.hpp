#pragma once

#include "../io/log.hpp"

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <utility>

namespace core {

class BumpArena {
private:
	char *const data;
	size_t capacity;
	size_t offset;

public:
	explicit BumpArena(size_t size) :
		data(static_cast<char*>(malloc(size))),
		capacity(size),
		offset(0)
	{ }
	~BumpArena() {
		free(this->data);
	}

public:
	template <typename T>
	T* alloc() {
		align_offset<T>();

		if (this->offset + sizeof(T) > this->capacity) {
			LOG_ERR("Out of memory.");
			free(this->data);

			std::abort();
			std::unreachable();
		} 

		T* ptr = reinterpret_cast<T*>(this->data + this->offset);
		this->offset += sizeof(T);
		return ptr;
	}

private:
	template <typename T>
	inline void align_offset() {
		this->offset = (this->offset + alignof(T) - 1) & ~(alignof(T) - 1);
	}
};

} // namespace core
