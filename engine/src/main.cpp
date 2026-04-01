#include "script/compiler.hpp"

#include <iostream>

static constexpr const char* path = "script.gby";

int main() {
	auto compiler = scr::Compiler(path, std::cout);

	if (!compiler.compile()) {
		return -1;
	}
}
