/*
 * This file defines the language specifications. 
 * It is written in C-compatible code.
*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include "../core/c/types.h"

// Differents lexeme for each commands.
#define CMD_UP_LEX "UP"
#define CMD_DOWN_LEX "DOWN"
#define CMD_RIGHT_LEX "RIGHT"
#define CMD_LEFT_LEX "LEFT"
#define CMD_GOTO_LEX "GOTO"
#define CMD_SPAWN_LEX "SPAWN"
#define CMD_DESPAWN_LEX "DESPAWN"
#define CMD_SHOW_LEX "SHOW"
#define CMD_WAIT_LEX "WAIT"
#define CMD_UPDATE_LEX "UPDATE"
#define CMD_COLLIDE_LEX "COLLIDE"

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
	CID_UPDATE,
	CID_COLLIDE,
} command_id_t;

// Built in types lexemes.
#define VOID_T_LEX "void"
#define INT_T_LEX "int"
#define FLOAT_T_LEX "float"
#define BOOL_T_LEX "bool"
#define STRING_T_LEX "str"
#define SPRITE_T_LEX "Sprite"

// Builtin property lexemes.
#define PROP_X_LEX 'x'
#define PROP_Y_LEX 'y'

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
