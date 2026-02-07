#pragma once

#include "block.hpp"

#include <ostream>
#include <vector>

namespace inst {

class BlockGroup {
private:
	std::vector<Block> blocks;

public:
	explicit BlockGroup();

	inline void add(const Block& block);
	inline bool write(std::ostream& stream);
};

} // namespace inst
