#include "fscript_mem.h"

#include <assert.h>

memory_t mem_new() {
	memory_t mem  = {
		.slots = vec_new(sizeof(block_t), block_freefn, block_copyfn),
		.stack = vec_new(sizeof(block_t), block_freefn, block_copyfn)
	};

	return mem;
}

void mem_store(memory_t* mem, const char* data, size_t size, word_t slot) {
	if (mem->slots.size > slot) {
		// Replace an already used slot.
		block_t* buf = (block_t*)vec_get(&mem->slots, slot);
		block_free(buf);
		block_init(buf, data, size);
	} else {
		// Push a new slot if it is valid.
		assert(slot == mem->slots.size);
		block_t* buf = (block_t*)vec_push_null(&mem->slots);
		block_init(buf, data, size);
	} 
}

const block_t* mem_load(memory_t* mem, word_t slot) {
	return (block_t*)vec_get(&mem->slots, slot);
}

void mem_push(memory_t* mem, const char* data, size_t size) {
	block_t* buf = (block_t*)vec_push_null(&mem->stack);
	block_init(buf, data, size);
}

block_t mem_pop(memory_t* mem) {
	block_t buf;
	vec_get_copy(&mem->stack, (char*)&buf, mem->stack.size);
	vec_pop(&mem->stack);
	return buf;
}
