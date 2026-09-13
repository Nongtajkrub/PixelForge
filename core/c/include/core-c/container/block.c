#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include "block.h"

#include <stdlib.h>
#include <memory.h>

block_t block_new(const char* data, size_t size) {
	block_t buf;

	buf.data = (char*)malloc(size);
	memcpy(buf.data, data, size);
	buf.size = size;

	return buf;
}

void block_init(block_t* block, const char* data, size_t size) {
	block->data = (char*)malloc(size);
	memcpy(block->data, data, size);
	block->size = size;
}

void block_copy(block_t* dest, block_t* src) {
	dest->size = src->size;
	dest->data = (char*)malloc(src->size);
	memcpy(dest->data, src->data, src->size);
}

void block_free(block_t* block) {
	free(block->data);
	block->data = NULL;
	block->size = 0;
}

void block_freefn(void* block) {
	block_free((block_t*)block);
}

void block_copyfn(void* dest, void* src) {
	block_copy((block_t*)dest, (block_t*)src);
}

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
