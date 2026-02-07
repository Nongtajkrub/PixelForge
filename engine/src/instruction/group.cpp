#include "group.hpp"
#include "block.hpp"

namespace inst {

inline void BlockGroup::add(const Block& block) {
	this->blocks.push_back(block);
}

inline bool BlockGroup::write(std::ostream& stream) {
	for (const Block& block : this->blocks) {
		if (!block.write(stream)) return false;
	}
	return true;
}

} // namespace inst
