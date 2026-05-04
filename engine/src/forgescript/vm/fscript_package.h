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
} fscript_pkg_t;

fscript_pkg_t fscript_pkg_load(char* bytes);
void fscript_pkg_destroy(fscript_pkg_t* pack);

block_t* fscript_pkg_cpool_get(const fscript_pkg_t* pack, word_t index);

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
