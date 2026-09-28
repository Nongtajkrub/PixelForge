#pragma once

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include <core-c/types.h>

typedef u16 word_t;

#define WORD_SIZE sizeof(word_t)

// IDs for each commands.
typedef enum : u8 {
	CID_UP,
	CID_DOWN,
	CID_RIGHT,
	CID_LEFT,
	CID_GOTO,
	CID_SPAWN,
	CID_DESPAWN,
	CID_SHOW,
	CID_WAIT,
	CID_COLLIDE,
} command_id_t;


#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
