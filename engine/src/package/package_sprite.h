#pragma once

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include <core-c/types.h>
#include <core-c/primitive_view.h>

#include "package_script.h"

typedef struct {
	u8_v width;
	u8_v height;
	u8* bitmap;
} pkg_sprite_looks_t;

size_t pkg_sprite_look_size(const pkg_sprite_looks_t* looks);
size_t pkg_sprite_look_height(const pkg_sprite_looks_t* looks);
u8 pkg_sprite_look_bitmap(const pkg_sprite_looks_t* looks, u8 x, u8 y);

typedef struct {
	pkg_sprite_looks_t looks;
	pkg_script_t script;
} pkg_sprite_t;

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
