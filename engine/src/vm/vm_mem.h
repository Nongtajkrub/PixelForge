#pragma once

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include <core-c/container/indexable_stack.h>
#include <core-c/allocator/bump_arena.h>
#include <core-c/container/block.h>
#include <core-c/types.h>

#include <sys/types.h>

#include "vm_specs.h"

typedef struct {
	size_t buf;
	size_t table;
} vmmem_size_config_t;

typedef struct {
	word_t size;
	char* ptr;
} vmmem_table_entry_t;

typedef struct {
	vmmem_size_config_t sizes;

	bump_arena_t buf;
	istack_t table;
} vmmem_t;

vmmem_t vmmem_new(mem_size_category_t size);

vmmem_table_entry_t vmmem_store(
	vmmem_t* mem, const char* data, size_t size, size_t slot);
vmmem_table_entry_t vmmem_load(const vmmem_t* mem, size_t slot);

void vmmem_free_back(vmmem_t* mem, size_t n);

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
