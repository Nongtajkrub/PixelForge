#pragma once

#include <core-cplusplus/types.hpp>

#include <vector>

namespace scr {

using CodePackage = std::vector<u8>;

CodePackage pack_script(
	const std::vector<u8>& cpool, const std::vector<u8>& code);

} // namespace scr
