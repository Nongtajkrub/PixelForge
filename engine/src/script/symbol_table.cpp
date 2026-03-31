#include "symbol_table.hpp"

#include <ranges>

namespace scr {

SymbolTable::SymbolTable() {
	// Global scope
	enter_scope(ScopeKind::GLOBAL);
}

bool SymbolTable::contains(UniversalIdType id) {
	for (const auto& scope : std::views::reverse(this->scopes)) {
		if (scope.table.contains(id)) {
			return true;
		}
	}
	return false;
}

bool SymbolTable::in_scope(ScopeKind kind) {
	for (const auto& scope : std::views::reverse(this->scopes)) {
		if (scope.kind == kind) {
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

std::optional<Ref<IdenAttr>> SymbolTable::lookup(UniversalIdType id) {
	for (auto& scope : std::views::reverse(this->scopes)) {
		if (auto it = scope.table.find(id); it != scope.table.end()) {
			return std::ref(it->second);
		}
	}
	return std::nullopt;
}

} // namespace scr
