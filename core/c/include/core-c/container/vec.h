#pragma once

#include <stddef.h>

#include "../types.h"

typedef struct {
	size_t capacity;
	size_t size;

	// Size of the element
	size_t esize;

	char* data;

	// Functions for interacting with the vector elements.
	callback_t freefn;
	binaryfn_t copyfn;
} vec_t;

vec_t vec_new(size_t esize, callback_t freefn, binaryfn_t copyfn);
void vec_init(vec_t* vec, size_t esize, callback_t freefn, binaryfn_t copyfn);
void vec_copy(vec_t* dest, vec_t* src);
void vec_destroy(vec_t* vec);

void vec_push(vec_t* vec, char* data);
// Push an uninitialized element into the vector and return the pointer to it. 
char* vec_push_null(vec_t* vec);
void vec_pop(vec_t* vec);

// Get a pointer to an element at a certain index.
char* vec_get(const vec_t* vec, size_t n);
// Copy an element at a certain index into a **preallocated** buffer.
void vec_get_copy(const vec_t* vec, char* dest, size_t n);

void vec_set(vec_t* vec, size_t n, char* data);

// Can be use in datastructure that accept a function (vec_t, ...).
void vec_freefn(void* vec);
void vec_copyfn(void* dest, void* src);
