#include "core-c/io/log.h"
#include <core-c/utilities/bump_arena.h>

#include <stdlib.h>
#include <stdalign.h>

bump_arena_t bumpa_new(size_t capacity) {
	return (bump_arena_t) {
		.data = (char*)malloc(capacity),
		.capacity = capacity,
		.offset = 0,
	};
}

void* bumpa_alloc(bump_arena_t* bumpa, size_t size, size_t align) {
	bumpa->offset = (bumpa->offset + align - 1) & ~(align - 1);

	if (bumpa->offset + size > bumpa->capacity) {
		LOG_ERR("Out of memory");
		return NULL;
	}

	void* ptr = bumpa->data + bumpa->offset;
	ptr = malloc(size);
	bumpa->offset += size;

	return ptr;
}
