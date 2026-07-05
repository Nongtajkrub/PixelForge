#include "vec_stream.h"

#include <stdbool.h>
#include <assert.h>

vec_stream_t vstream_make(vec_t* vec) {
	return (vec_stream_t) {
		.vec = vec,
		.cusor = 0,
	};
}

const char* vstream_advance(vec_stream_t* stream) {
	assert(!vstream_end(stream));
	stream->cusor++;
	return vec_get(stream->vec, stream->cusor - 1);
}

const char* vstream_peek(const vec_stream_t* stream) {
	return vec_get(stream->vec, stream->cusor);
}

bool vstream_end(const vec_stream_t* stream) {
	return stream->cusor == stream->vec->size;
}

size_t vstream_cursor(const vec_stream_t *stream) {
	return stream->cusor;
}
