#include "byte.hpp"

#include "../global.hpp"

#include <span>
#include <iostream>

namespace io {

namespace byte {

void output_stdout(const std::span<u8> bytes) {
	for (const u8 byte : bytes) {
		std::cout << std::hex << static_cast<int>(byte) << ' ';
	}
	std::cout << '\n';
}

} // namespace byte

} // namespace io
