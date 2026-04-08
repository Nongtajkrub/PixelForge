#include "vm_def.h"

const char* op_to_str(opcode_t op) {
	switch (op) {
	case OP_CONST: return "CONST";
	case OP_LOAD: return "LOAD";
	case OP_STORE: return "STORE";
	case OP_CMD: return "CMD";
	}
}


bool op_have_operand(opcode_t op) {
	switch (op) {
	case OP_CONST:
	case OP_LOAD:
	case OP_STORE: 
	case OP_CMD:
		return true;
	}
}
