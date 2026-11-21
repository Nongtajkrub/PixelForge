#include "vm_mem.h"

#include "vm_specs.h"
#include "../state.h"

#include <core-c/container/indexable_stack.h>
#include <core-c/allocator/bump_arena.h>
#include <core-c/io/log.h>

#include <assert.h>
#include <memory.h>
#include <stdlib.h>

extern engine_state_t eng_state;

static vmmem_size_config_t resolve_size_config(mem_size_category_t category) {
	switch (category) {
	case MEM_SIZE_S:
		return (vmmem_size_config_t) {
			.buf = 512,
			.table = 64
		};
	case MEM_SIZE_MD:
		return (vmmem_size_config_t) {
			.buf = 1024,
			.table = 128
		};
	case MEM_SIZE_L:
		return (vmmem_size_config_t) {
			.buf = 2048,
			.table = 256
		};
	case MEM_SIZE_XL:
		return (vmmem_size_config_t) {
			.buf = 4096,
			.table = 512
		};

	}
}

vmmem_t vmmem_new(mem_size_category_t size) {
	vmmem_size_config_t sizes = resolve_size_config(size);

	bump_arena_t buf = 
		bump_arena_new_with_xalloc(
			bump_arena_xalloc_fn, &eng_state.allocator, sizes.buf); 

	assert(bump_arena_is_valid(&buf));

	istack_t table = 
		FSTACK_NEW_XALLOC(
			sizes.table,
			vmmem_table_entry_t, bump_arena_xalloc_fn, &eng_state.allocator);

	assert(istack_is_valid(&table));

	return (vmmem_t) {
		.sizes = size,
		.buf = buf,
		.table = table,
	};
}

static vmmem_table_entry_t push_slot(
	vmmem_t* mem, const char* data, size_t size) {
	if (!bump_arena_is_allocable(&mem->buf, size)) {
		BUG("Vm out of memory");
		exit(1);
	}

	char* buf = BUMP_ARENA_ALLOC(&mem->buf, char*);
	assert(buf != NULL);
	memcpy(buf, data, size);

	vmmem_table_entry_t entry = {
		.ptr = buf,
		.size = size,
	};
	istack_push(&mem->table, (char*)&entry);

	return entry;
}

static vmmem_table_entry_t overwrite_slot(
	vmmem_t* mem, const char* data, size_t size, size_t slot) {
	vmmem_table_entry_t* entry = istack_get(&mem->table, slot);
	
	if (size != entry->size) {
		BUG("Invalid object size for memory slot");
		exit(1);
	}

	memcpy(entry->ptr, data, size);

	return *entry;
}

vmmem_table_entry_t vmmem_store(
	vmmem_t* mem, const char* data, size_t size, size_t slot) {
	if (slot > mem->table.stk_size) {
		BUG("Invalid memory storage slot");
		exit(1);
	}

	if (slot == mem->table.stk_size) {
		return push_slot(mem, data, size);
	} else {
		return overwrite_slot(mem, data, size, slot);
	}
}

void vmmem_free_back(vmmem_t* mem, size_t n) {
	assert(n < mem->table.stk_size);

	for (size_t i = 0; i < n; i++) {
		istack_pop_rm(&mem->table);
		bump_arena_free_back(&mem->buf);
	}
}
