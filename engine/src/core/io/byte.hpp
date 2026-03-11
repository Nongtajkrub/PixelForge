#pragma once

#include "../../global.hpp"

#include <span>

namespace io {

namespace byte {

void output_stdout(const std::span<u8> bytes);

} // namespace byte

} // namespace io
