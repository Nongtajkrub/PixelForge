#pragma once

#include <core-cplusplus/types.hpp>

#include <span>

namespace scr {

using CodePackage = std::vector<u8>;

CodePackage pack_script(std::span<const u8> cpool, std::span<const u8> code);

} // namespace scr
