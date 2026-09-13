#pragma once

#include <core-cplusplus/types.hpp>

#include <optional>
#include <ostream>
#include <filesystem>

namespace scr {

class Compiler {
private:
	std::filesystem::path src_path;
	std::ostream& err_stream;

public:
	Compiler(const std::filesystem::path& src_path, std::ostream& err_stream) :
		src_path(src_path), err_stream(err_stream)
	{ }

	std::optional<std::vector<u8>> compile();
};

}; // namespace scr
