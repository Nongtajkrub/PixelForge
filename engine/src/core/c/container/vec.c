#include "vec.h"

#include <stdlib.h>
#include <assert.h>
#include <memory.h>

#define INIT_VEC_CAP 16

vec_t vec_new(size_t esize) {
	return (vec_t) {
		.capacity = INIT_VEC_CAP,
		.size = 0,
		.esize = esize,
		.data = (char*)malloc(INIT_VEC_CAP * esize)
	};
}

void vec_push(vec_t* vec, char* data) {
	if (vec->size == vec->capacity) {
		vec->capacity *= 2;
		vec->data = (char*)realloc(vec->data, vec->capacity * vec->esize);
	}

	memcpy(vec->data + (vec->size * vec->esize), data, vec->esize);
	vec->size++;
}

void* vec_push_null(vec_t* vec) {
	if (vec->size == vec->capacity) {
		vec->capacity *= 2;
		vec->data = (char*)realloc(vec->data, vec->capacity * vec->esize);
	}

	vec->size++;
	return (vec->data + ((vec->size - 1) * vec->esize));
}

void vec_pop(vec_t* vec) {
	assert(vec->esize != 0);
	vec->size--;
}

char* vec_get(vec_t* vec, size_t n) {
	return vec->data + (vec->esize * n);
}

block_t vec_get_copy(vec_t* vec, size_t n) {
	return block_new(vec_get(vec, n), vec->esize);
}
