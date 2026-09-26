#include "package_sprite.h"

#include <core-c/primitive_view.h>

#include <assert.h>

size_t pkg_sprite_look_width(const pkg_sprite_looks_t* looks) {
	return read_u8_v(looks->width);
}

size_t pkg_sprite_look_height(const pkg_sprite_looks_t* looks) {
	return read_u8_v(looks->height);
}

u8 pkg_sprite_look_bitmap(const pkg_sprite_looks_t* looks, u8 x, u8 y) {
	assert(x < pkg_sprite_look_width(looks) && y < pkg_sprite_look_height(looks));
	return (looks->bitmap[(y << 1) + (x >> 3)] >> (7 - (x & 7))) & 1;
}
