#pragma once

#include "../core/cplusplus/types.hpp"

#include <vector>
#include <span>

namespace scr {

std::vector<u8> pack(std::span<const u8> cpool, std::span<const u8> code);

} // namespace scr
