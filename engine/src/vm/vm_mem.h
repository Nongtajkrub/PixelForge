#pragma once

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include <core-c/container/fixed_stack.h>
#include <core-c/allocator/bump_arena.h>
#include <core-c/container/block.h>
#include <core-c/types.h>

#include <sys/types.h>

#include "vm_specs.h"

typedef struct {
	size_t buf;
	size_t table;
} vm_mem_size_config_t;

typedef struct {
	word_t size;
	char* data;
} vm_mem_table_item_t;

typedef struct {
	vm_mem_size_config_t sizes;

	bump_arena_t buf;
	fixed_stack_t table;
} vm_mem_t;

vm_mem_t vm_mem_new(mem_size_category_t size);

vm_mem_table_item_t vm_mem_store(
	vm_mem_t* mem, char* data, size_t size, size_t slot);
vm_mem_table_item_t vm_mem_load(const vm_mem_t* mem, size_t slot);

void vm_mem_del(vm_mem_t* mem, size_t n);

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
