#include "symbol_table.hpp"

#include <ranges>

namespace scr {

SymbolTable::SymbolTable() :
	iden_interner([this]() -> IdentifierId {
		return this->iden_id_generator.generate(); 
	})
{
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

bool SymbolTable::contains(IdentifierId id) {
	for (const auto& scope : std::views::reverse(this->scopes)) {
		if (const auto it = scope.table.find(id); 
				it != scope.table.end() && it->second.top().in_scope) {
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

std::optional<Ref<IdenAttr>> SymbolTable::lookup(IdentifierId id) {
	for (auto& scope : std::views::reverse(this->scopes)) {
		if (auto it = scope.table.find(id);
				it != scope.table.end() && it->second.top().in_scope) {
			return std::ref(it->second.top());
		}
	}
	return std::nullopt;
}

} // namespace scr
