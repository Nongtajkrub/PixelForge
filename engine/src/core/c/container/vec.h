#pragma onec

#include <stddef.h>

#include "block.h"

typedef struct {
	size_t capacity;
	size_t size;

	// Size of the element
	size_t esize;

	char* data;
} vec_t;

vec_t vec_new(size_t esize);

void vec_push(vec_t* vec, char* data);
void* vec_push_null(vec_t* vec);
void vec_pop(vec_t* vec);

char* vec_get(vec_t* vec, size_t n);
block_t vec_get_copy(vec_t* vec, size_t n);
