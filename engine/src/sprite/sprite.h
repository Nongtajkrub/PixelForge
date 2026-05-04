#pragma once

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include "../forgescript/vm/fscript_package.h"
#include "../core/c/container/bitmap.h"
#include "../core/c/container/vec.h"

typedef struct {
	bitmap2d_t looks;
	fscript_pkg_t script;

	vec_t instances;
} sprite_t;

sprite_t spr_load(char* data);

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
