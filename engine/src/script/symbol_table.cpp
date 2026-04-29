#include "symbol_table.hpp"
#include "specs.h"

#include <ranges>

namespace scr {

static TypeAttr construct_void_attr() {
	return (TypeAttr) {
		.id = TypeId::VOID,
		.is_value = false,
	};
}

static TypeAttr construct_int_attr() {
	return (TypeAttr) {
		.id = TypeId::INT,
		.is_value = true,
	};
}

static TypeAttr construct_float_attr() {
	return (TypeAttr) {
		.id = TypeId::FLOAT,
		.is_value = true,
	};
}

static TypeAttr construct_bool_attr() {
	return (TypeAttr) {
		.id = TypeId::BOOL,
		.is_value = true,
	};
}

static TypeAttr construct_str_attr() {
	return (TypeAttr) {
		.id = TypeId::STR,
		.is_value = true,
	};
}

static TypeAttr construct_sprite_attr() {
	return (TypeAttr) {
		.id = TypeId::SPRITE,
		.is_value = true,
	};
}

SymbolTable::SymbolTable() :
	iden_interner([this]() -> IdentifierId {
		return this->iden_id_generator.generate(); 
	})
{
	// Global scope
	enter_scope(ScopeKind::GLOBAL);

	// Setup types.
	this->types.ty_int = 
		&new_identifier_global(
			intern(VOID_T_LEX),
			IdenAttr(construct_void_attr()))->get_data<TypeAttr>();
	this->types.ty_int = 
		&new_identifier_global(
			intern(INT_T_LEX),
			IdenAttr(construct_int_attr()))->get_data<TypeAttr>();
	this->types.ty_float =
		&new_identifier_global(
			intern(FLOAT_T_LEX),
			IdenAttr(construct_float_attr()))->get_data<TypeAttr>();
	this->types.ty_bool =
		&new_identifier_global(
			intern(BOOL_T_LEX),
			IdenAttr(construct_bool_attr()))->get_data<TypeAttr>();
	this->types.ty_str =
		&new_identifier_global(
			intern(STR_T_LEX),
			IdenAttr(construct_str_attr()))->get_data<TypeAttr>();
	this->types.ty_sprite = 
		&new_identifier_global(
			intern(SPRITE_T_LEX),
			IdenAttr(construct_sprite_attr()))->get_data<TypeAttr>();

	// Setup builtin identifiers.
	new_identifier_global(
		intern("bi_key_up"), IdenAttr(this->types.ty_bool, VarAttr()));
	new_identifier_global(
		intern("bi_key_down"), IdenAttr(this->types.ty_bool, VarAttr()));
	new_identifier_global(
		intern("bi_key_right"), IdenAttr(this->types.ty_bool, VarAttr()));
	new_identifier_global(
		intern("bi_key_left"), IdenAttr(this->types.ty_bool, VarAttr()));

	new_identifier_global(
		intern("bi_random_x"), IdenAttr(this->types.ty_int, FuncAttr()));
	new_identifier_global(
		intern("bi_random_y"), IdenAttr(this->types.ty_int, FuncAttr()));
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
