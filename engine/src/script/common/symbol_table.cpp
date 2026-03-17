#include "symbol_table.hpp"

namespace scr {

SymbolTable::SymbolTable() {
	// Global scope
	enter_scope();
}

} // namespace scr
