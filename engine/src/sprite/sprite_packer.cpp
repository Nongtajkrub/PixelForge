#include "sprite_packer.hpp"

#include <core-cplusplus/io/byte_io.hpp>

namespace sprite {

SpritePackage pack_sprite(std::vector<bool> data, u8 width, u8 height) {
	SpritePackage buf;
	auto io = core::BytesBufferWriter(buf);
}

}; // namespace sprite
