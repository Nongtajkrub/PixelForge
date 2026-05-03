#pragma once

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include "../../core/c/container/block.h"
#include "../../core/c/container/vec.h"
#include "../fscript_specs.h"

#include <stddef.h>

typedef struct {
	// Const pool data.
	struct {
		size_t size;
		vec_t data;
	} cpool;

	// Instructions data.
	struct {
		struct {
			size_t size;
			vec_t data;
		} main;

		struct {
			size_t size;
			vec_t data;
		} func;
	} code;
} package_t;

package_t pkg_deserialize(char* bytes);
void pkg_destroy(package_t* pack);

block_t* pkg_cpool_get(const package_t* pack, word_t index);

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
