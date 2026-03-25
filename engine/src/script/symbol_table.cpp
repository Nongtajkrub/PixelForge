#include "symbol_table.hpp"

#include <ranges>

namespace scr {

SymbolTable::SymbolTable() {
	// Global scope
	enter_scope();
}

bool SymbolTable::contains(UniversalIdType id) {
	for (const auto& scope : std::views::reverse(this->table)) {
		if (scope.contains(id)) {
			return true;
		}
	}
	return false;
}

UniversalIdType SymbolTable::intern_iden(const std::string& iden) {
	auto [it, inserted] = iden_interner.try_emplace(iden, UniversalIdType{});

	if (inserted) {
		it->second = IncrementalIdGen::generate();
	}

	return it->second;
}

std::optional<IdenAttr> SymbolTable::lookup(UniversalIdType id) {
	for (const auto& scope : std::views::reverse(this->table)) {
		if (auto it = scope.find(id); it != scope.end()) {
			return it->second;
		}
	}
	return std::nullopt;
}

} // namespace scr
