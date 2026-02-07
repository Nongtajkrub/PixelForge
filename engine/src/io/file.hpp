#pragma once

#include <optional>
#include <string>
#include <filesystem>

namespace io {

namespace file {

std::optional<std::string> load_str(const std::filesystem::path& path);

} // namespace file

} // namespace io
