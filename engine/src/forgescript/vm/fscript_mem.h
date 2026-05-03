#pragma once

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include "../../core/c/container/block.h"
#include "../../core/c/container/vec.h"

#include "../fscript_specs.h"

typedef struct {
	vec_t slots;
	vec_t stack;
} memory_t;

memory_t mem_new();

void mem_store(memory_t* mem, const char* data, size_t size, word_t slot);
const block_t* mem_load(memory_t* mem, word_t slot);

void mem_push(memory_t* mem, const char* data, size_t size);
block_t mem_pop(memory_t* mem);

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
