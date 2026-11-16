#pragma once

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include "xalloc.h"

#include <core-c/utilities/primitive_view.h>

#include <stddef.h>

#define BUMP_ARENA_ALLOC(arena_, type_) \
	bump_arena_alloc(arena_, sizeof(type_), alignof(TYPE))

typedef struct {
	xalloc_info_single_t xalloc;

	/*
	 * Buffer Structure:
	 * [(data, metadata), ...]
	 *
	 * - Data: The memory block returned to the user.
	 * - Metadata: An `arena_meta_t` storing the offset from previous data back.
	 */	
	char* data;

	size_t capacity;
	size_t offset;
} bump_arena_t;

bump_arena_t bump_arena_new(size_t capacity);
bump_arena_t bump_arena_new_with_xalloc(
	xallocfn fn, void* allocator, size_t capacity);

bool bump_arena_is_valid(const bump_arena_t* arena);
size_t bump_arena_is_allocable(const bump_arena_t* arena, size_t size);

void* bump_arena_alloc(bump_arena_t* arena, size_t size, size_t align);
void* bump_arena_xalloc_fn(void* arena, size_t size, size_t align);
void bump_arena_free(bump_arena_t* arena);
void bump_arena_free_back(bump_arena_t* arena);

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
