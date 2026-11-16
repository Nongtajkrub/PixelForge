#pragma once

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include <core-c/allocator/bump_arena.h>

#include "package/package_game.h"

#define ENGINE_MEM_SIZE 307200

typedef struct {
	bool init;

	char mem[ENGINE_MEM_SIZE];
	bump_arena_t allocator;

	pkg_game_t pkg;
} engine_state_t;

typedef enum : u8 {
	ENG_STATUS_OK,

	ENG_STATUS_FAIL_INITIALIZE,
	ENG_STATUS_ALREADY_INITIALIZE,

	ENG_STATUS_VM_MEM_EXCESSIVE_POP,
} engine_status_t;

engine_status_t engine_init();
void engine_exit(engine_status_t status);

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
