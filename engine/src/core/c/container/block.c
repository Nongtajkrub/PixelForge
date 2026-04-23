#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include "block.h"

#include <stdlib.h>
#include <memory.h>

block_t block_new(char* data, size_t size) {
	block_t buff;

	buff.data = (char*)malloc(size);
	memcpy(buff.data, data, size);
	buff.size = size;

	return buff;
}

void block_free(block_t* block) {
	free(block->data);
	block->data = NULL;
	block->size = 0;
}

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
