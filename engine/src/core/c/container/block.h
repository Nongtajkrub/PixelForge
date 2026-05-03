#pragma once

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include <stddef.h>

typedef struct {
	char* data;
	size_t size;
} block_t;

block_t block_new(const char* data, size_t size);
void block_init(block_t* block, const char* data, size_t size);

void block_copy(block_t* dest, block_t* src);
void block_free(block_t* block);

// Can be use in datastructure that accept a function (vec_t, ...).
void block_freefn(void* block);
void block_copyfn(void* dest, void* src); 

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
