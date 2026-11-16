#include "vm_mem.h"

#include "vm_specs.h"
#include "../state.h"

#include <core-c/container/fixed_stack.h>
#include <core-c/allocator/bump_arena.h>
#include <core-c/io/log.h>

#include <assert.h>
#include <memory.h>

extern engine_state_t eng_state;

static vm_mem_size_config_t resolve_size_config(mem_size_category_t category) {
	switch (category) {
	case MEM_SIZE_S:
		return (vm_mem_size_config_t) {
			.buf = 512,
			.table = 64
		};
	case MEM_SIZE_MD:
		return (vm_mem_size_config_t) {
			.buf = 1024,
			.table = 128
		};
	case MEM_SIZE_L:
		return (vm_mem_size_config_t) {
			.buf = 2048,
			.table = 256
		};
	case MEM_SIZE_XL:
		return (vm_mem_size_config_t) {
			.buf = 4096,
			.table = 512
		};

	}
}

vm_mem_t vm_mem_new(mem_size_category_t size) {
	vm_mem_size_config_t sizes = resolve_size_config(size);

	bump_arena_t buf = 
		bump_arena_new_with_xalloc(
			bump_arena_xalloc_fn, &eng_state.allocator, sizes.buf); 

	assert(bump_arena_is_valid(&buf));

	fixed_stack_t table = 
		FSTACK_NEW_XALLOC(
			sizes.table,
			vm_mem_table_item_t, bump_arena_xalloc_fn, &eng_state.allocator);

	assert(fstack_is_valid(&table));

	return (vm_mem_t) {
		.sizes = size,
		.buf = buf,
		.table = table,
	};
}
