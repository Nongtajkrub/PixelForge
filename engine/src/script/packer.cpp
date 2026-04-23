#include "packer.hpp"

#include "../core/cplusplus/io/byte_io.hpp"

#include <vector>

namespace scr {

using namespace core;

std::vector<u8> pack(std::span<const u8> cpool, std::span<const u8> code) {
	std::vector<u8> buff;
	buff.reserve(cpool.size() + code.size());

	push_bytes(buff, reinterpret_cast<const char*>(cpool.data()), cpool.size());
	push_bytes(buff, reinterpret_cast<const char*>(code.data()), code.size());

	return buff;
}

} // namespace scr
