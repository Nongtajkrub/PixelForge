#include "file.hpp"

#include <optional>
#include <fstream>
#include <iterator>

namespace core {

std::optional<std::string> fload_str(const std::filesystem::path& path) {
	auto fd = std::ifstream(path);
	if (!fd.is_open()) {
		return std::nullopt;
	}

	const auto content = std::string(
		std::istreambuf_iterator<char>(fd), std::istreambuf_iterator<char>());

	fd.close();
	return content;
}

} // namespace core
