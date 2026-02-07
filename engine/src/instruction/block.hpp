#pragma once

#include <ostream>
#include <vector>
#include <span>

#include "../global.hpp"
#include "type.hpp"

namespace inst {

struct Block {
	BlockType type;
	std::vector<u32> operands;

	explicit Block(BlockType type, std::span<const u32> operands = { }) :
		type(type),
		operands(operands.begin(), operands.end())
	{ }

	bool write(std::ostream& stream) const;
};

}; // namespace inst
