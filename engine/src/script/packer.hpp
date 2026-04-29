#pragma once

#include "../core/cplusplus/types.hpp"
#include "code_generator.hpp"
#include "const_pool.hpp"

#include <vector>

namespace scr {

std::vector<u8> pack(const ConstPool& cpool, const CodeGenerator& code_gen);

} // namespace scr
