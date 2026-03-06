#pragma once

#include <cstddef>

namespace scr {

struct Location {
	size_t row;
	size_t col;

	explicit Location(size_t row, size_t col) :
		row(row), col(col)
	{ }
};

} // namespace scr
