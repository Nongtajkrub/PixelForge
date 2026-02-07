#pragma once

#include "../global.hpp"

namespace inst {

enum class BlockType : u8 {
	// movement
	GOTO = 0,
	FORWARD = 1,
	BACKWARD = 2,
};

};
