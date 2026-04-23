/*
 * This file defines the shared contract between the compiler and the vm.
 * It contains all low-level language definitions required by both sides.
 * This file is written in C-compatible code.
*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include "../specs.h"

#include <stdbool.h>

typedef word_t instruction_t;

typedef enum : instruction_t {
	OP_NOP,
	OP_END,
	OP_CONST,
	OP_CONST_DIRECT,
	OP_LOAD,
	OP_LOAD_STACK,
	OP_LOAD_PROP,
	OP_STORE,
	OP_STORE_STACK,
	OP_PUSH,
	OP_POP,
	OP_CALL,
	OP_RETURN,
	OP_COMMAND,
	OP_ADD,
	OP_MINUS,
	OP_MULTIPLY,
	OP_DIVIDE,
	OP_COMPARE_EQUAL,
	OP_COMPARE_GEATER,
	OP_COMPARE_LESS,
	OP_COMPARE_NOT_EQUAL,
	OP_COMPARE_GEATER_EQUAL,
	OP_COMPARE_LESS_EQUAL,
	OP_JMP,
	OP_JMP_TRUE,
	OP_JMP_FALSE,
} opcode_t;

const char* op_to_str(opcode_t op);
bool op_have_operand(opcode_t op);

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
