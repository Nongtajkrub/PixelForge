#include "bytecode.hpp"

namespace scr {

const char* op_to_str(OpCode op) {
	switch (op) {
	case OpCode::CONST: return "CONST";
	}
}

} // namespace scr
