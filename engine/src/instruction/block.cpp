#include "block.hpp"

#include <ostream>
#include <vector>

namespace inst {

bool Block::write(std::ostream& stream) const {
	const auto cast_type = static_cast<u32>(this->type);
	stream.write(reinterpret_cast<const char*>(&cast_type), sizeof(u8));
	stream.write(
		reinterpret_cast<const char*>(this->operands.data()),
		this->operands.size() * sizeof(u32));
	return !stream.fail();
}

}; // namespace inst
