#include <core-c/allocator/xalloc.h>

void* xalloc_static_buf(void* buf, size_t size, size_t align) {
	return buf;
}
