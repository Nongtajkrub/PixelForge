#pragma once

#include "diagnostic.hpp"
#include "symbol_table.hpp"

#include <ostream>
#include <filesystem>

namespace scr {

class Compiler {
private:
	SymbolTable symbols = SymbolTable();

	std::filesystem::path src_path;
	std::ostream& err_stream;

public:
	Compiler(const std::filesystem::path& src_path, std::ostream& err_stream) :
		src_path(src_path), err_stream(err_stream)
	{ }

	bool compile();

private:
	inline void emit(DiagnosticKind kind) {
		Diagnostic(kind).emit(this->err_stream);
	}
};

}; // namespace scr
