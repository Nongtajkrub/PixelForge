#pragma once

#include <cstdint>
#include <iostream>
#include <span>

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using f32 = float;

inline void output_bytes(std::span<const u8> bytes) {
	for (const u8 byte : bytes) {
		std::cout << std::hex << static_cast<int>(byte) << ' ';
	}
	std::cout << '\n';
}
