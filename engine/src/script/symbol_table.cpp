#include "symbol_table.hpp"

#include <ranges>

namespace scr {

SymbolTable::SymbolTable() {
	// Global scope
	enter_scope(ScopeKind::GLOBAL);

	// Setup builtin identifiers.
	new_identifier(
		intern_iden("bi_key_up"), IdenAttr(IdenKind::VAR ,TokenKind::BOOL_T));
	new_identifier(
		intern_iden("bi_key_down"), IdenAttr(IdenKind::VAR ,TokenKind::BOOL_T));
	new_identifier(
		intern_iden("bi_key_right"), IdenAttr(IdenKind::VAR ,TokenKind::BOOL_T));
	new_identifier(
		intern_iden("bi_key_left"), IdenAttr(IdenKind::VAR ,TokenKind::BOOL_T));

	new_identifier(
		intern_iden("bi_random_x"), IdenAttr(IdenKind::FUNC ,TokenKind::INT_T));
	new_identifier(
		intern_iden("bi_random_y"), IdenAttr(IdenKind::FUNC ,TokenKind::INT_T));
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
