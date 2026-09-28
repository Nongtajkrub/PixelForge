#pragma once

#include <core-cplusplus/types.hpp>

#include <boost/dynamic_bitset.hpp>

#include <cassert>
#include <vector>

namespace sprite {

class SpriteLooks {
public:
	explicit SpriteLooks(u8 width, u8 height);
	~SpriteLooks() = default;

	inline void set_pixel(u8 x, u8 y, bool value) {
		assert(x < this->width && y < this->height);
		bitmap[(y * width) + x] = value;
	}

	std::vector<u8> serialize();

private:
	u8 width;
	u8 height;

	boost::dynamic_bitset<u8> bitmap;
};

}; // namespace sprite
