#pragma once

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include <core-c/utilities/bump_arena.h>

#include <stddef.h>
#include <stdbool.h>

typedef struct {
	char* ptr;

	size_t size;
	size_t e_size;
} mem_sector_t;

mem_sector_t memsec_new(void* ptr, size_t size, size_t e_size);

void* memsec_get(const mem_sector_t* memsec, size_t n); 

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
