#pragma once

#include <core-cplusplus/types.hpp>

#include <vector>
#include <span>

namespace sprite {

using SpritePackage = std::vector<u8>;

SpritePackage pack_sprite(std::span<u8> looks, std::span<u8> script);

}; // namespace sprite
