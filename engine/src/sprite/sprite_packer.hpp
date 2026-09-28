#pragma once

#include <core-cplusplus/types.hpp>

#include <vector>

namespace sprite {

using SpritePackage = std::vector<u8>;

SpritePackage pack_sprite(std::vector<bool> data, u8 width, u8 height);

}; // namespace sprite
