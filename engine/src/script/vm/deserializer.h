#pragma once

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include "../../core/c/container/vec.h"

#include <stddef.h>

typedef struct {
	size_t size;

	// Const pool data.
	struct {
		struct {
			size_t size;
			vec_t map;
		} addr;

		struct {
			size_t size;
			vec_t data;
		} entries;
	} cpool;

	// Instructions data.
	struct {
		size_t size;

		vec_t inst;
		vec_t func;
	} code;
} package_t;

package_t deserialize(char* bytes, size_t size);

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
