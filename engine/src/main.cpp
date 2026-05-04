#include "forgescript/fscript_compiler.hpp"

#include <iostream>

static constexpr const char* path = "script.gby";

int main() {
	auto compiler = scr::Compiler(path, std::cout);

	const auto code = compiler.compile();
	if (!code) return -1;
}
