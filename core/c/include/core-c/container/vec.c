#include "vec.h"

#include <stdlib.h>
#include <assert.h>
#include <memory.h>

#define INIT_VEC_CAP 16

vec_t vec_new(size_t esize, callback_t freefn, binaryfn_t copyfn) {
	return (vec_t) {
		.capacity = INIT_VEC_CAP,
		.size = 0,
		.esize = esize,
		.data = (char*)malloc(INIT_VEC_CAP * esize),
		.freefn = freefn,
		.copyfn = copyfn,
	};
}

void vec_init(vec_t* vec, size_t esize, callback_t freefn, binaryfn_t copyfn) {
	vec->capacity = INIT_VEC_CAP;
	vec->size = 0;
	vec->esize = esize;
	vec->data = (char*)malloc(INIT_VEC_CAP * esize);
	vec->freefn = freefn;
	vec->copyfn = copyfn;
}

void vec_copy(vec_t* dest, vec_t* src) {
	dest->capacity = src->capacity;
	dest->size = src->size;
	dest->esize = src->esize;
	dest->data = (char*)malloc(src->capacity * src->esize);
	dest->freefn = src->freefn;
	dest->copyfn = src->copyfn;

	// Copy data over.
	if (src->copyfn) {
		char* ptr = dest->data;

		for (size_t i = 0; i < src->size; i++) {
			vec_get_copy(src, ptr, i);
			ptr += src->esize;
		}
	} else {
		memcpy(dest->data, src->data, src->capacity * src->esize);
	}
}

void vec_destroy(vec_t* vec) {
	if (vec->freefn) {
		// Free individual elements if a freefn is provided.
		for (size_t i = 0; i < vec->size; i++) {
			vec->freefn(vec_get(vec, i));
		}
	}
	free(vec->data);

	vec->capacity = 0;
	vec->size = 0;
	vec->esize = 0;
	vec->data = NULL;
	vec->freefn = NULL;
}

void vec_push(vec_t* vec, char* data) {
	if (vec->size == vec->capacity) {
		vec->capacity *= 2;
		vec->data = (char*)realloc(vec->data, vec->capacity * vec->esize);
	}

	if (vec->copyfn) {
		vec->copyfn(vec->data + (vec->size * vec->esize), data);
	} else {
		memcpy(vec->data + (vec->size * vec->esize), data, vec->esize);
	}
	vec->size++;
}

char* vec_push_null(vec_t* vec) {
	if (vec->size == vec->capacity) {
		vec->capacity *= 2;
		vec->data = (char*)realloc(vec->data, vec->capacity * vec->esize);
	}

	vec->size++;
	return (vec->data + ((vec->size - 1) * vec->esize));
}

void vec_pop(vec_t* vec) {
	assert(vec->size != 0);

	if (vec->freefn) {
		vec->freefn(vec_get(vec, vec->size - 1));
	} 

	vec->size--;
}

char* vec_get(const vec_t* vec, size_t n) {
	assert(vec->size > n);
	return vec->data + (vec->esize * n);
}

void vec_get_copy(const vec_t* vec, char* dest, size_t n) {
	assert(dest != NULL);

	if (vec->copyfn) {
		vec->copyfn(dest, vec_get(vec, n));
	} else {
		memcpy(dest, vec_get(vec, n), vec->esize);
	}
}

void vec_set(vec_t* vec, size_t n, char* src) {
	assert(vec->size > n);
	memcpy(vec->data + (n * vec->esize), src, vec->esize);
}

void vec_freefn(void* vec) {
	vec_destroy((vec_t*)vec);
}

void vec_copyfn(void* dest, void* src) {
	vec_copy((vec_t*)dest, (vec_t*)src);
}
