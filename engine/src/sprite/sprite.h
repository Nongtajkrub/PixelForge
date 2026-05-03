#pragma once

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include "../core/c/container/vec.h"
#include "../core/c/types.h"

typedef struct {
	vec_t sprites;
} sprite_manager_t; 

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
