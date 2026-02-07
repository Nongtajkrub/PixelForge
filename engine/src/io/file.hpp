#pragma once

#include <optional>
#include <string>
#include <filesystem>

namespace file {

std::optional<std::string> load_str(const std::filesystem::path& path);

}
