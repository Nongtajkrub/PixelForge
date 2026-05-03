#include "forgescript/vm/fscript_interpreter.h"
#include "forgescript/fscript_compiler.hpp"

#include <iostream>

static constexpr const char* path = "script.gby";

int main() {
	auto compiler = scr::Compiler(path, std::cout);

	const auto code = compiler.compile();
	if (!code) return -1;

	/*
	package_t pkg = pkg_deserialize((char*)code->data());
	interpret(&pkg);
	*/
}
