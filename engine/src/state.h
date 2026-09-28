#pragma once

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include "package/package_game.h"

typedef struct {
	pkg_game_t pkg;
} engine_state_t;

engine_state_t engine_init(engine_state_t* engine);

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
