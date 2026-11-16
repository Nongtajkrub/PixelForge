#include "state.h"

#include "core-c/allocator/xalloc.h"
#include <core-c/allocator/bump_arena.h>

#include <string.h>

engine_state_t eng_state; 

engine_status_t engine_init() {
	if (eng_state.init) return ENG_STATUS_ALREADY_INITIALIZE;
	eng_state.init = true;

	memset(eng_state.mem, 0, ENGINE_MEM_SIZE);
	eng_state.allocator =
		bump_arena_new_with_xalloc(
			xallocfn_static_buf, eng_state.mem, ENGINE_MEM_SIZE);

	if (!bump_arena_is_valid(&eng_state.allocator))
		return ENG_STATUS_FAIL_INITIALIZE;

	return ENG_STATUS_OK;
}
