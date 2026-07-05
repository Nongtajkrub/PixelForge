#pragma once

#include "../types.hpp"

#include <type_traits>
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <vector>
#include <span>

namespace core {

void bytes_output(std::span<const u8> bytes, std::ostream& stream);

void push_bytes(std::vector<u8>& dest, const char* bytes, size_t size);

template<typename T>
requires std::is_trivially_copyable_v<T>
size_t push_bytes(std::vector<u8>& dest, T data) {
	const auto location = dest.size();
	const auto bytes = reinterpret_cast<const char*>(&data);
	dest.insert(dest.end(), bytes, bytes + sizeof(T));
	return location;
}

template<typename T>
requires std::is_trivially_copyable_v<T>
void replace_bytes(std::vector<u8>& dest, size_t n, T data) {
	const auto bytes = reinterpret_cast<const char*>(&data);
	std::copy(bytes, bytes + sizeof(T), dest.begin() + n);
}

class BytesBufferWriter {
public:
	struct HoleToken {
		size_t index;
		size_t size;

		HoleToken(size_t index, size_t size) :
			index(index), size(size)
		{ }
	};

private:
	std::vector<u8>& bytes;

public:
	BytesBufferWriter(std::vector<u8>& bytes) :
		bytes(bytes)
	{ }

	// Push data into the buffer.
	template<typename T>
	requires std::is_trivially_copyable_v<T>
	void write(T data) {
		push_bytes<T>(this->bytes, data);
	}

	// Push an empty hole into the buffer and return the token to it.
	template<typename T>
	requires std::is_trivially_copyable_v<T>
	HoleToken hole() {
		const auto token = HoleToken(this->bytes.size(), sizeof(T));
		push_bytes<T>(this->bytes, 0);
		return token;
	}

	template<typename T>
	requires std::is_trivially_copyable_v<T>
	void fill(T data, HoleToken token) {
		assert(token.size == sizeof(T));
		replace_bytes(this->bytes, token.index, data);
	}

	std::vector<u8>& get_buf() {
		return this->bytes;
	}
};

} // namespace core
