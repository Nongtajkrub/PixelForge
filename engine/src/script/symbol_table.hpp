#pragma once

#include "../core/cursor_stack.hpp"
#include "../core/incremental_id.hpp"
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

enum class IdenKind : u8 {
	VAR,
	FUNC,
};

struct IdenAttr {
	IdenKind kind;
	
	// Identifier data types.
	TokenKind type;
	bool in_scope = true;

	// Argument type list for wehn kind is FUNC.
	std::optional<std::vector<TokenKind>> arg_types = std::nullopt;

	IdenAttr() = default;
	explicit IdenAttr(IdenKind kind, TokenKind type) :
		kind(kind), type(type)
	{
		if (kind == IdenKind::FUNC) {
			arg_types = std::vector<TokenKind>{};
		}
	
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

public:
	SymbolTable();

	// Check whether a symbol exist.
	bool contains(IdentifierId id);

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
			attr_stack.top().in_scope = false;
		}
		
		this->scopes.rewind();
	}

	inline ScopeKind this_scope() {
		return this->scopes.top().kind;
	}

	// Add a new or replace symbol.
	inline IdenAttr& new_identifier(IdentifierId id, IdenAttr attr) {
		auto [it, _] = this->scopes.top().table.try_emplace(id);
		it->second.push(attr);
		return it->second.top();
	}

	// Add a new or replace global symbol.
	inline IdenAttr& new_identifier_global(IdentifierId id, IdenAttr attr) {
		auto [it, _] = this->scopes.bottom().table.try_emplace(id);
		it->second.push(attr);
		return it->second.top();
	}
};

} // namespace scr
