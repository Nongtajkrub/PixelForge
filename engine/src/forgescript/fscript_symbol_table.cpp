#include "fscript_symbol_table.hpp"

#include "fscript_specs.h"

#include <ranges>

namespace scr {

std::vector<TypeAttr*> TypeAttr::get_prop_types() const {
	std::vector<TypeAttr*> types;
	types.reserve(this->properties.size());

	for (const auto& prop : this->properties) {
		types.push_back(prop.type);
	}

	return types;
}

SymbolTable::SymbolTable() :
	iden_interner([this]() -> IdentifierId {
		return this->iden_id_generator.generate(); 
	})
{
	enter_scope(ScopeKind::GLOBAL);

	// Setup types.
	this->types.ty_void = 
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
	this->types.ty_sprite = 
		&new_identifier_global(
			intern(SPRITE_T_LEX),
			IdenAttr(construct_sprite_attr()))->get_data<TypeAttr>();
}

void SymbolTable::enter_scope(ScopeKind kind, IdenAttr* owner) {
	auto [scope, inserted] = this->scopes.try_emplace_back(kind, owner);

	// If no new scope was inserted (cursor reused an existing slot),
	// construct the scope manually.
	if (!inserted) {
		scope.kind = kind;
		scope.owner = owner;
	}
}

void SymbolTable::leave_scope() {
	// Ensure do not pop global scope.
	assert(this->scopes.size() > 1);

	// Move all identifier in the current scope out of scope.
	for (auto& [_, attr_stack] : this->scopes.top().table) {
		auto& attr = attr_stack.top();

		if (attr.in_scope) {
			attr.in_scope = false;
		}
	}
	
	this->scopes.top().stack_offset_gen.reset();
	this->scopes.rewind();
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

IdenAttr* SymbolTable::new_identifier(
	IdentifierId id, IdenAttr attr, Scope& scope) {

	if (attr.kind_is<VarAttr>()) {
		attr.get_data<VarAttr>().offset = scope.stack_offset_gen.generate();
	} else if (attr.kind_is<TypeAttr>()) {
		attr.get_data<TypeAttr>().id = id;
	}

	auto [it, _] = scope.table.try_emplace(id);
	it->second.emplace(std::move(attr));
	return &it->second.top();
}

IdenAttr* SymbolTable::lookup(IdentifierId id, Scope& scope) {
	if (auto it = scope.table.find(id);
			it != scope.table.end() && it->second.top().in_scope) {
		return &it->second.top();
	}
	return nullptr;
}

TypeAttr SymbolTable::construct_void_attr() {
	return (TypeAttr) {
		.is_value = false,
	};
}

TypeAttr SymbolTable::construct_int_attr() {
	return (TypeAttr) {
		.is_value = true,
	};
}

TypeAttr SymbolTable::construct_float_attr() {
	return (TypeAttr) {
		.is_value = true,
	};
}

TypeAttr SymbolTable::construct_bool_attr() {
	return (TypeAttr) {
		.is_value = true,
	};
}

TypeAttr SymbolTable::construct_sprite_attr() {
	return (TypeAttr) {
		.is_value = true,
		.properties = {
			PropAttr(intern("x"), this->types.ty_int),
			PropAttr(intern("y"), this->types.ty_int),
		}
	};
}

} // namespace scr
