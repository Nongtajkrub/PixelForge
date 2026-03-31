#pragma once

#include "../core/iterable_stack.hpp"
#include "../core/incremental_id.hpp"
#include "token.hpp"

#include <cassert>
#include <cstddef>
#include <optional>
#include <unordered_map>

namespace scr {

using namespace core;

enum class IdenKind : u8 {
	VAR,
	FUNC,
};

struct IdenAttr {
	IdenKind kind;
	
	// Identifier data types.
	TokenKind type;

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
	std::unordered_map<UniversalIdType, IdenAttr> table;

	Scope() = default;
	Scope(ScopeKind kind) : 
		kind(kind), table()
	{ }
};
 
class SymbolTable {
private:
	// Assigns a unique ID to each string; identical strings share the same ID.
	std::unordered_map<std::string, UniversalIdType> iden_interner; 

	// Identifier table store in a stack to manage scope.
	IterableStack<Scope> scopes;

public:
	SymbolTable();

	// Intern an identifier to ID.
	UniversalIdType intern_iden(const std::string& iden);

	// Check whether a symbol exist.
	bool contains(UniversalIdType id);

	bool in_scope(ScopeKind kind);

	// Look up a symbol and return the reference to its attr.
	std::optional<Ref<IdenAttr>> lookup(UniversalIdType id);

	inline void enter_scope(ScopeKind kind) {
		this->scopes.emplace_back(kind);
	}

	inline void leave_scope() {
		// Ensure do not pop global scope.
		assert(this->scopes.size() > 1);
		this->scopes.pop();
	}

	inline ScopeKind this_scope() {
		return this->scopes.top().kind;
	}

	// Add a new or replace symbol.
	inline IdenAttr& new_identifier(UniversalIdType id, IdenAttr attr) {
		auto [it, _] = this->scopes.top().table.emplace(id, attr);
		return it->second;
	}

	// Add a new or replace global symbol.
	inline void new_identifier_global(UniversalIdType id, IdenAttr attr) {
		this->scopes.bottom().table.emplace(id, attr);
	}
};

} // namespace scr
