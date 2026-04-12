#include "instruction.h"

const char* op_to_str(opcode_t op) {
	switch (op) {
	case OP_BEGIN: return "BEGIN";
	case OP_END: return "END";
	case OP_CONST: return "CONST";
	case OP_LOAD: return "LOAD";
	case OP_STORE: return "STORE";
	case OP_COMMAND: return "CMD";
	case OP_CALL: return "CALL";
	case OP_ADD: return "ADD";
	case OP_MINUS: return "MIN";
	case OP_MULTIPLY: return "MUL";
	case OP_DIVIDE: return "DIV";
	case OP_COMPARE_EQUAL: return "COMPARE_EQUAL";
	case OP_COMPARE_GEATER: return "COMPARE_GREATER";
	case OP_COMPARE_LESS: return "COMPARE_LESS";
	case OP_COMPARE_NOT_EQUAL: return "COMPARE_NOT_EQUAL";
	case OP_COMPARE_GEATER_EQUAL: return "COMPARE_GREATER_EQUAL";
	case OP_COMPARE_LESS_EQUAL: return "COMPARE_LESS_EQUAL";
	case OP_JMP: return "JMP";
	case OP_JMP_TRUE: return "JMP_TRUE";
	case OP_JMP_FALSE: return "JMP_FALSE";
	}
}

bool op_have_operand(opcode_t op) {
	switch (op) {
	case OP_CONST:
	case OP_LOAD:
	case OP_STORE: 
	case OP_COMMAND:
	case OP_CALL:
	case OP_JMP:
	case OP_JMP_TRUE:
	case OP_JMP_FALSE:
		return true;
	case OP_BEGIN:
	case OP_END:
	case OP_ADD:
	case OP_MINUS:
	case OP_MULTIPLY:
	case OP_DIVIDE:
	case OP_COMPARE_EQUAL:
	case OP_COMPARE_GEATER:
	case OP_COMPARE_LESS:
	case OP_COMPARE_NOT_EQUAL:
	case OP_COMPARE_GEATER_EQUAL:
	case OP_COMPARE_LESS_EQUAL:
		return false;
	}
}
