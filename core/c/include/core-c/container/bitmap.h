#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct {
	size_t width;
	size_t height;

	bool* data;
} bitmap2d_t;

bool bit2d_set(bool v, size_t x, size_t y);
bool bit2d_get(size_t x, size_t y);
