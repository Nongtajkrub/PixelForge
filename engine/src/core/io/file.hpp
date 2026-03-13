#pragma once

#include <optional>
#include <string>
#include <filesystem>

namespace core {

std::optional<std::string> fload_str(const std::filesystem::path& path);

} // namespace io
