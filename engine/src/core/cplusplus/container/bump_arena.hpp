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

	// Prevent copying
	BumpArena(const BumpArena&) = delete;
	BumpArena& operator=(const BumpArena&) = delete;

public:
	template <typename T, typename ...Args>
	T* alloc(Args&&... args) {
		align_offset<T>();

		if (this->offset + sizeof(T) > this->capacity) {
			LOG_ERR("Out of memory.");
			free(this->data);

			std::abort();
		} 

		void* ptr = this->data + this->offset;
		this->offset += sizeof(T);

		return new (ptr) T(std::forward<Args>(args)...);
	}

private:
	template <typename T>
	inline void align_offset() {
		this->offset = (this->offset + alignof(T) - 1) & ~(alignof(T) - 1);
	}
};

} // namespace core
