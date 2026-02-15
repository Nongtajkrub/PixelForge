#pragma once

#include <cstddef>

namespace scr {

struct Location {
	size_t line;

	explicit Location(size_t line) :
		line(line)
	{ }
};

} // namespace scr
