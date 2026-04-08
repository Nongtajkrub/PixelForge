/*
 * This file defines the shared contract between the compiler and the vm.
 * It contains all low-level language definitions required by both sides.
 * This file is written in C-compatible code.
*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include "../global.h"

#include <stdbool.h>

// Differents lexeme for each commands.
#define CMD_UP_LEX "UP"
#define CMD_DOWN_LEX "DOWN"
#define CMD_RIGHT_LEX "RIGHT"
#define CMD_LEFT_LEX "LEFT"
#define CMD_GOTO_LEX "GOTO"
#define CMD_SPAWN_LEX "SPAWN"
#define CMD_DESPAWN_LEX "DESPAWN"
#define CMD_SHOW_LEX "SHOW"
#define CMD_UPDATE_LEX "UPDATE"
#define CMD_COLLIDE_LEX "COLLIDE"

typedef enum : u8 {
	CID_UP,
	CID_DOWN,
	CID_RIGHT,
	CID_LEFT,
	CID_GOTO,
	CID_SPAWN,
	CID_DESPAWN,
	CID_SHOW,
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

typedef u16 instruction_t;

typedef enum : instruction_t {
	OP_CONST,
	OP_LOAD,
	OP_STORE,
	OP_CMD,
} opcode_t;

const char* op_to_str(opcode_t op);
bool op_have_operand(opcode_t op);

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
