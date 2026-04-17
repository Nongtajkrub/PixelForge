#pragma once

#include "../types.hpp"

#include <cstddef>
#include <string>

namespace core {

class UniqueStringGenerator {
public:
	std::string generate();

	UniqueStringGenerator(const char* prefix = "") :
		prefix(prefix)
	{ }
	
private:
	std::string prefix = "";

	u32 counter = 0;
};

} // namespace core
