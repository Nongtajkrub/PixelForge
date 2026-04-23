#include "byte_io.hpp"

#include <span>
#include <iostream>

namespace core {

void bytes_output(const std::span<const u8> bytes, std::ostream& stream) {
	for (const u8 byte : bytes) {
		stream <<  static_cast<int>(byte) << ' ';
	}
	std::cout << '\n';
}

void push_bytes(std::vector<u8>& dest, const char* bytes, size_t size) {
	dest.insert(dest.end(), bytes, bytes + size);
}

} // namespace core
