#pragma once

#include "../core/cplusplus/types.hpp"
#include "fscript_code_generator.hpp"
#include "fscript_const_pool.hpp"

#include <vector>

namespace scr {

std::vector<u8> pack(const ConstPool& cpool, const CodeGenerator& code_gen);

} // namespace scr
