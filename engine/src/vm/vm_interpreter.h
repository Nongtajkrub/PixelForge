#pragma once

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include "vm_mem.h"
#include "vm_stack.h"

#include "../package/package_sprite.h"

typedef struct {
	pkg_script_cpool_t* cpool;
	pkg_sprite_t* sprite;

	u32 pc;
	vm_mem_t mem;
	vm_stack_t stack;
} vm_interpreter_t;

vm_interpreter_t vminter_new(pkg_script_cpool_t* cpool, pkg_sprite_t* sprite);

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
