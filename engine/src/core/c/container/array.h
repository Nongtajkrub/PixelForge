#pragma once

#include "../types.h"

#include <stddef.h>

typedef struct {
	size_t size;

	// Sizeof element in the array.
	size_t esize;

	char* data;

	// Functions for interacting wih the elements.
	callback_t freefn;
	binaryfn_t copyfn;
} array_t;

array_t arr_new(size_t size, size_t esize, callback_t freefn, binaryfn_t copyfn);

void arr_set(array_t* arr, char* data, size_t n);
