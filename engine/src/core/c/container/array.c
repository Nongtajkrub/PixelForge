#include "array.h"

#include <stdlib.h>

array_t arr_new(size_t size, size_t esize, callback_t freefn, binaryfn_t copyfn) {
	return (array_t) {
		.size = size,
		.esize = esize,
		.data = (char*)malloc(size * esize),
		.freefn = freefn,
		.copyfn = copyfn,
	};
}

void arr_set(array_t* arr, char* data, size_t n) {
	if (arr->copyfn) {
		arr->copyfn(arr->data + (arr->esize * n), data);
	}
}
