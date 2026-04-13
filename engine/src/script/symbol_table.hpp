#pragma once

#include "../core/cplusplus/container/cursor_stack.hpp"
#include "../core/cplusplus/utilities/incremental_id.hpp"
#include "../core/cplusplus/utilities/free_list_id.hpp"
#include "../core/cplusplus/utilities/id_interner.hpp"
#include "../core/cplusplus/utilities/variant.hpp"
#include "token.hpp"

#include <unordered_map>
#include <optional>
#include <concepts>
#include <cassert>
#include <cstddef>
#include <utility>
#include <stack>

namespace scr {

using namespace core;
using IdentifierId = u32;
using VariableSlot = u16;
using ArgStackIndex = u16;

struct VarAttr {
	VariableSlot slot;
};

struct ArgAttr{
	ArgStackIndex index;
};

struct FuncAttr {
	std::vector<TokenKind> args_types;
};

struct IdenAttr {
	bool in_scope = true;
	TokenKind type;

	Variant<VarAttr, ArgAttr, FuncAttr> data;

	IdenAttr() = default;
	template <typename T>
	requires std::same_as<T, VarAttr> 
		|| std::same_as<T, ArgAttr> || std::same_as<T, FuncAttr>
	IdenAttr(TokenKind type, T&& attr) :
		type(type), data(std::forward<T>(attr))
	{
		assert(token_is_type(type));
	}
};

enum class ScopeKind {
	GLOBAL,
	IF,
	FUNC,
	LOOP,
};

struct Scope {
	ScopeKind kind;

	// IdenAttr store in a stack to support identifier shadowing.
	std::unordered_map<IdentifierId, std::stack<IdenAttr>> table;

	Scope() = default;
	Scope(ScopeKind kind) : 
		kind(kind), table()
	{ }

};
 
class SymbolTable {
private:
	// Identifier table stored in a cursor stack to manage scopes while
	// preserving identifier attributes, ensuring references remain valid.	
	CursorStack<Scope> scopes;

	// Assigns a unique slot to each identifier, identical share same slot.
	IdInterner<std::string, IdentifierId> iden_interner; 

	// Generate ID for identifiers.
	IncrementalIdGen<IdentifierId> iden_id_generator = 
		IncrementalIdGen<IdentifierId>(0);

	// Generate slot for variable identifiers.
	FreeListIdGen<VariableSlot> var_slot_generator =
		FreeListIdGen<VariableSlot>(0);

	// Generate slot for function arguments.
	IncrementalIdGen<ArgStackIndex> stack_index_generator =
		IncrementalIdGen<ArgStackIndex>(0);

public:
	SymbolTable();

	// Check whether a symbol exist.
	bool contains(IdentifierId id);

	// Check whether symbol exist in the current scope only.
	bool contains_in_scope(IdentifierId id);

	bool in_scope(ScopeKind kind);

	// Look up a symbol and return the reference to its attr.
	std::optional<Ref<IdenAttr>> lookup(IdentifierId id);

	// Intern an identifier to ID.
	inline IdentifierId intern_iden(const std::string& iden) {
		auto [id, _] = this->iden_interner.intern(iden);
		return id;
	}

	inline void enter_scope(ScopeKind kind) {
		auto [scope, inserted] = this->scopes.try_emplace_back(kind);

		// If no new scope was inserted (cursor reused an existing slot),
		// update the scope kind manually.		
		if (!inserted) {
			scope.kind = kind;
		}

		this->stack_index_generator.reset();
	}

	inline void leave_scope() {
		// Ensure do not pop global scope.
		assert(this->scopes.size() > 1);
		this->stack_index_generator.reset();

		// Move all identifier in the current scope out of scope.
		for (auto& [_, attr_stack] : this->scopes.top().table) {
			auto& attr = attr_stack.top();

			if (attr.in_scope) {
				attr.in_scope = false;

				// Free the slot used.
				if (attr.data.is<VarAttr>()) {
					this->var_slot_generator.free(attr.data.get<VarAttr>().slot);
				}
			}
		}
		
		this->scopes.rewind();
	}

	inline ScopeKind this_scope() {
		return this->scopes.top().kind;
	}

	// Add a new or replace symbol.
	inline IdenAttr& new_identifier(IdentifierId id, IdenAttr attr) {
		return new_identifier(id, attr, this->scopes.top());
	}

	// Add a new or replace global symbol.
	inline IdenAttr& new_identifier_global(IdentifierId id, IdenAttr attr) {
		return new_identifier(id, attr, this->scopes.bottom());
	}

private:
	inline IdenAttr& new_identifier(
		IdentifierId id, IdenAttr attr, Scope& scope) {
		if (attr.data.is<VarAttr>()) {
			attr.data.get<VarAttr>().slot = this->var_slot_generator.generate();
		} else if (attr.data.is<ArgAttr>()) {
			attr.data.get<ArgAttr>().index =
				this->stack_index_generator.generate();
		} else if (attr.data.is<FuncAttr>()) {
			attr.data.get<FuncAttr>().args_types = std::vector<TokenKind>{};
		}

		auto [it, _] = scope.table.try_emplace(id);
		it->second.emplace(std::move(attr));
		return it->second.top();
	}
};

} // namespace scr
