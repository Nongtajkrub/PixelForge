#pragma once

#include "../types.hpp"

#include <cstddef>
#include <span>
#include <type_traits>
#include <vector>

namespace core {

void bytes_output(std::span<const u8> bytes, std::ostream& stream);

void push_bytes(std::vector<u8>& dest, const char* bytes, size_t size);

template<typename T>
requires std::is_trivially_copyable_v<T>
void push_bytes(std::vector<u8>& dest, T data) {
	const auto bytes = reinterpret_cast<const char*>(&data);
	dest.insert(dest.end(), bytes, bytes + sizeof(T));
}

} // namespace core
