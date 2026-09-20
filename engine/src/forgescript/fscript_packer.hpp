#pragma once

#include <core-cplusplus/types.hpp>

#include "fscript_code_generator.hpp"
#include "fscript_const_pool.hpp"

#include <vector>

namespace scr {

using CodePackage = std::vector<u8>;

CodePackage pack(const ConstPool& cpool, const CodeGenerator& code_gen);

} // namespace scr
