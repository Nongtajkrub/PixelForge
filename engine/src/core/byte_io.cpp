#include "byte_io.hpp"

#include <span>
#include <iostream>

namespace core {

void byte_output_stdout(const std::span<u8> bytes) {
	for (const u8 byte : bytes) {
		std::cout << std::hex << static_cast<int>(byte) << ' ';
	}
	std::cout << '\n';
}

} // namespace core
