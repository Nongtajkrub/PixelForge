#include "package_script.h"

#include <assert.h>
#include <core-c/primitive_view.h>

instruction_t pkg_script_subroutine_inst(const char* routine, size_t n) {
	return (instruction_t)read_u16_v((u16_v)&routine[n * 2]);
}
