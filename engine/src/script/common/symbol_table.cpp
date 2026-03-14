#include "symbol_table.hpp"

namespace scr {

SymbolTable::SymbolTable() {
	// Global scope
	push_scope();
}

} // namespace scr
