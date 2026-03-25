#pragma once

#include "../core/stack/iterable.hpp"
#include "../core/id/incremental.hpp"
#include "token.hpp"

#include <cassert>
#include <cstddef>
#include <optional>
#include <unordered_map>

namespace scr {

using namespace core;

struct IdenAttr {
	// Identifier data types.
	TokenKind type;

	IdenAttr() = default;
	explicit IdenAttr(TokenKind type) :
		type(type)
	{
		assert(token_is_type(type));
	}
};
 
class SymbolTable {
private:
	// Assigns a unique ID to each string; identical strings share the same ID.
	std::unordered_map<std::string, UniversalIdType> iden_interner; 

	// Identifier table store in a stack to manage scope.
	IterableStack<std::unordered_map<UniversalIdType, IdenAttr>> table;

public:
	SymbolTable();

	// Intern an identifier to ID.
	UniversalIdType intern_iden(const std::string& iden);

	// Check whether a symbol exist.
	bool contains(UniversalIdType id);

	// Check whether a symbol exist.
	std::optional<IdenAttr> lookup(UniversalIdType id);

	inline void enter_scope() {
		this->table.emplace_back();
	}

	inline void leave_scope() {
		// Ensure do not pop global scope.
		assert(this->table.size() > 1);
		this->table.pop();
	}

	// Add a new or replace symbol.
	inline void new_identifier(UniversalIdType id, IdenAttr attr) {
		this->table.top().emplace(id, attr);
	}

	// Add a new or replace global symbol.
	inline void new_identifier_global(UniversalIdType id, IdenAttr attr) {
		this->table.bottom().emplace(id, attr);
	}
};

} // namespace scr
