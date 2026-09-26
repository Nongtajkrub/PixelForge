#pragma once

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include <stddef.h>

typedef struct {
	char *const data;

	size_t capacity;
	size_t offset;
} bump_arena_t;

bump_arena_t bumpa_new(size_t capacity);

void* bumpa_alloc(bump_arena_t* bumpa, size_t size, size_t align);

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
