#pragma once

#include "../global.hpp"

#include <cstddef>
#include <optional>

namespace scr {

enum class OpCode {
	CONST,
};

struct Instruction {
	OpCode op;
	std::optional<u16> operand = std::nullopt;

	Instruction(OpCode op) :
		op(op)
	{ }
	Instruction(OpCode op, u16 operand) :
		op(op), operand(operand)
	{ }
};

const char* op_to_str(OpCode op);

}; // namespace scr
