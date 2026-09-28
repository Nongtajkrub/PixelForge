#include "package_script.h"

#include <assert.h>
#include <core-c/primitive_view.h>

static inline u16_v traverse_metadata(u16_v ptr, size_t n) {
	size_t i = 0;

	while (i != n) {
		ptr += read_u16_v(ptr);
		i++;
	}

	return ptr;
}

char* pkg_script_cpool_data(const pkg_script_cpool_t* cpool, size_t n) {
	assert(n < cpool->size);
	return traverse_metadata(cpool->data, n);
}

char* pkg_script_cpool_data_nometa(const pkg_script_cpool_t* cpool, size_t n) {
	assert(n < cpool->size);
	return traverse_metadata(cpool->data, n) + sizeof(u16);
}

instruction_t pkg_script_subroutine_inst(const char* routine, size_t n) {
	return (instruction_t)read_u16_v((u16_v)&routine[n * 2]);
}

pkg_script_subroutine_t pkg_script_subroutine(
	const pkg_script_subroutine_list_t* list, size_t n) {
	assert(n < list->size);

	u16_v ptr = traverse_metadata(list->routines, n);

	return (pkg_script_subroutine_t) {
		.size = read_u16_v(ptr),
		.inst = ptr + sizeof(u16),
	};
}
