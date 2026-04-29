#pragma once

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include "../../core/c/container/vec.h"

typedef struct {
	vec_t slots;
} memory_t;

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
