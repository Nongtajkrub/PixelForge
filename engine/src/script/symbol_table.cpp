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
		intern_iden("bi_key_up"), IdenAttr(TokenKind::BOOL_T, VarAttr()));
	new_identifier(
		intern_iden("bi_key_down"), IdenAttr(TokenKind::BOOL_T, VarAttr()));
	new_identifier(
		intern_iden("bi_key_right"), IdenAttr(TokenKind::BOOL_T, VarAttr()));
	new_identifier(
		intern_iden("bi_key_left"), IdenAttr(TokenKind::BOOL_T, VarAttr()));

	new_identifier(
		intern_iden("bi_random_x"), IdenAttr(TokenKind::INT_T, FuncAttr()));
	new_identifier(
		intern_iden("bi_random_y"), IdenAttr(TokenKind::INT_T, FuncAttr()));
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

bool SymbolTable::contains_in_scope(IdentifierId id) {
	auto& scope = this->scopes.top();
	if (const auto it = scope.table.find(id); 
			it != scope.table.end() && it->second.top().in_scope) {
		return true;
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

IdenAttr* SymbolTable::get_scope_owner() {
	for (const auto& scope : std::views::reverse(this->scopes)) {
		if (scope.owner != nullptr) {
			return scope.owner;
		}
	}
	return nullptr;
}

IdenAttr* SymbolTable::lookup(IdentifierId id) {
	for (auto& scope : std::views::reverse(this->scopes)) {
		if (auto attr = lookup(id, scope)) {
			return attr;
		}
	}
	return nullptr;
}

IdenAttr* SymbolTable::lookup(IdentifierId id, Scope& scope) {
	if (auto it = scope.table.find(id);
			it != scope.table.end() && it->second.top().in_scope) {
		return &it->second.top();
	}
	return nullptr;
}

} // namespace scr
