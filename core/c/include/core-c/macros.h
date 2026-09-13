#pragma once

#include <assert.h>

#define TODO() assert(true && "TODO!")

#define CPY_AND_MV_PTR(DEST, SRC, SIZE)                                        \
	memcpy(DEST, SRC, SIZE);                                                   \
	SRC += SIZE 
