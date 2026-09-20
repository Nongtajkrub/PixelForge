#pragma once

#include "fscript_token.hpp"

#include <cstddef>
#include <optional>
#include <vector>

namespace scr {

using TokenBuffer = std::vector<Token>;

std::optional<TokenBuffer> lex(const std::string& src, std::ostream& err_stream);

} // namespace scr
