#include <core-c/container/mem_sector.h>

#include <core-c/utilities/bump_arena.h>

mem_sector_t memsec_new(void* ptr, size_t size, size_t e_size) {
	return (mem_sector_t) {
		.ptr = ptr,
		.size = size,
		.e_size = e_size,
	};
}

void* memsec_get(const mem_sector_t* memsec, size_t n) {
	return memsec->ptr + (n * memsec->e_size); 
}
