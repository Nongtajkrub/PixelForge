#pragma once

#include "../container/vec.h"

#include <stddef.h>
#include <stdbool.h>

typedef struct {
	vec_t* vec;
	size_t cusor;
} vec_stream_t;

vec_stream_t vstream_make(vec_t* vec);

const char* vstream_advance(vec_stream_t* stream);
const char* vstream_peek(const vec_stream_t* stream);

bool vstream_end(const vec_stream_t* stream);
