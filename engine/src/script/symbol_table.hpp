#pragma once

#include "../core/cursor_stack.hpp"
#include "../core/incremental_id.hpp"
#include "../core/free_list_id.hpp"
#include "../core/interner.hpp"
#include "token.hpp"

#include <cassert>
#include <cstddef>
#include <optional>
#include <unordered_map>
#include <stack>

namespace scr {

using namespace core;
using IdentifierId = u32;
using IdentifierSlot = u16;

enum class IdenKind : u8 {
	VAR,
	FUNC,
};

struct IdenAttr {
	IdenKind kind;
	
	bool in_scope = true;
	TokenKind type;

	// Where the identifier is store when kind is VAR.
	std::optional<IdentifierSlot> slot;

	// Argument type list for when kind is FUNC.
	std::optional<std::vector<TokenKind>> arg_types = std::nullopt;

	IdenAttr() = default;
	explicit IdenAttr(IdenKind kind, TokenKind type) :
		kind(kind), type(type)
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
	IncrementalIdGen<IdentifierId> iden_id_generator = 
		IncrementalIdGen<IdentifierId>(0);

	// Generate slot for variable identifiers.
	FreeListIdGen<IdentifierSlot> slot_generator =
		FreeListIdGen<IdentifierSlot>(0);

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
	}

	inline void leave_scope() {
		// Ensure do not pop global scope.
		assert(this->scopes.size() > 1);

		// Move all identifier in the current scope out of scope.
		for (auto& [_, attr_stack] : this->scopes.top().table) {
			auto& attr = attr_stack.top();

			attr.in_scope = false;

			// Free the slot for variable.
			if (attr.kind == IdenKind::VAR) {
				this->slot_generator.free(*attr.slot);
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
		if (attr.kind == IdenKind::VAR) {
			attr.slot = this->slot_generator.generate();
		}
		if (attr.kind == IdenKind::FUNC) {
			attr.arg_types = std::vector<TokenKind>{};
		}

		auto [it, _] = scope.table.try_emplace(id);
		it->second.emplace(std::move(attr));
		return it->second.top();
	}
};

} // namespace scr
