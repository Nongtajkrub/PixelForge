#include <engine/forgescript/fscript_compiler.hpp>

#include <iostream>

static constexpr const char* path = "script.gby";

int main() {
	auto compiler = scr::Compiler(path, std::cout);

	auto code = compiler.compile();
	if (!code) return -1;
}
