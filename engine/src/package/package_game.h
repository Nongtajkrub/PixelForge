#pragma once

#include "package_script.h"
#include "package_sprite.h"
#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

typedef struct {
	pkg_sprite_t sprite;
	pkg_script_cpool_t cpool;
} pkg_game_t;

pkg_game_t pkg_game_new(const char* pkg);

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
