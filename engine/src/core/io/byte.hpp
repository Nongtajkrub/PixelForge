#pragma once

#include "../../global.hpp"

#include <span>

namespace core {

void bytes_output_stdout(const std::span<u8> bytes);

} // namespace core
