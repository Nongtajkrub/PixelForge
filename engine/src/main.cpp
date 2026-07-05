#include "forgescript/fscript_compiler.hpp"
#include "forgescript/vm/fscript_interpreter.h"
#include "forgescript/vm/fscript_package.h"

#include <iostream>

static constexpr const char* path = "script.gby";

int main() {
	auto compiler = scr::Compiler(path, std::cout);

	auto code = compiler.compile();
	if (!code) return -1;

	fscript_pkg_t pkg = fscript_pkg_load((char*)((*code).data()));

	fscript_interpret(&pkg);
}
