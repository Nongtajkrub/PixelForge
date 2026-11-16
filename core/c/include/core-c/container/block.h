#pragma once

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include <stddef.h>

typedef struct {
	size_t size;
	char* data;
} block_t;

block_t block_invalid();
bool block_is_valid(const block_t* block);

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
