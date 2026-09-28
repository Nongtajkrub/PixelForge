#include "sprite_looks.hpp"

#include <core-cplusplus/io/byte_io.hpp>

#include <boost/dynamic_bitset.hpp>

#include <iterator>

namespace sprite {

SpriteLooks::SpriteLooks(u8 width, u8 height) :
	width(width), height(height), bitmap(width * height) 
{ }

std::vector<u8> SpriteLooks::serialize() {
	std::vector<u8> buf;
	auto io = core::BytesBufferWriter(buf);

	io.write<u8>(this->width);
	io.write<u8>(this->height);
	boost::to_block_range(this->bitmap, std::back_inserter(io.get_buf()));

	return buf;
}

}; // namespace sprite
