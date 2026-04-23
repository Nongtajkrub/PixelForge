#pragma once

#include <stddef.h>

typedef struct {
	char* data;
	size_t size;
} block_t;

block_t block_new(char* data, size_t size);
void block_free(block_t* block);
