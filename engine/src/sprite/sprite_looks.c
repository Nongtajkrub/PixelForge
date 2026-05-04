#pragma once

#include "../core/c/macros.h"
#include "sprite_looks.h"

#include <cstring>
#include <memory.h>
#include <stdlib.h>

bitmap2d_t spr_looks_load(char* data) {
	bitmap2d_t buf = { 0 };
	char* ptr = data;

	CPY_AND_MV_PTR(&buf.width, ptr, sizeof(buf.width));
	CPY_AND_MV_PTR(&buf.height, ptr, sizeof(buf.height));

	const size_t size = buf.width * buf.height;
	buf.data = (bool*)malloc(size);
	CPY_AND_MV_PTR(buf.data, ptr, size);

	return buf;
}
