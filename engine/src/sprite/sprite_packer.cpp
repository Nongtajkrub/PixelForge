#include "sprite_packer.hpp"

#include <core-cplusplus/io/byte_io.hpp>

namespace sprite {

SpritePackage pack_sprite(std::span<u8> looks, std::span<u8> script) {
	SpritePackage buf;
	auto io = core::BytesBufferWriter(buf);

	io.extend(looks);
	io.extend(script);

	return buf;
}

}; // namespace sprite
