#pragma once

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include <core-c/types.h>
#include <core-c/primitive_view.h>

#include "../forgescript/vm/fscript_instruction.h"

typedef struct {
	size_t_v size;
	char* data;
} pkg_script_cpool_t;

char* pkg_script_cpool_data(const pkg_script_cpool_t* cpool, size_t n);

typedef struct {
	size_t_v size;
	char* inst;
} pkg_script_subroutine_t;

typedef struct {
	size_t size;
	char* routines;
} pkg_script_subroutine_list_t;

instruction_t pkg_script_subroutine_inst(const char* routine, size_t n);

typedef struct {
	size_t_v size;

	pkg_script_subroutine_t main;
	pkg_script_subroutine_list_t func;
	pkg_script_subroutine_list_t update;
} pkg_script_t;

pkg_script_subroutine_t pkg_script_subroutine(
	const pkg_script_subroutine_list_t* list, size_t n);

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
