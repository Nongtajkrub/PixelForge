#pragma once

#include "../core/stack/iterable.hpp"
#include "../core/id/incremental.hpp"
#include "token.hpp"

#include <cassert>
#include <cstddef>
#include <functional>
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
	std::optional<std::vector<TokenKind>> arg_type_list = std::nullopt;

	IdenAttr() = default;
	explicit IdenAttr(IdenKind kind, TokenKind type) :
		kind(kind), type(type)
	{
		if (kind == IdenKind::FUNC) {
			arg_type_list = std::vector<TokenKind>{};
		}
	
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

	// Look up a symbol and return the reference to its attr.
	std::optional<Ref<IdenAttr>> lookup(UniversalIdType id);

	inline void enter_scope() {
		this->table.emplace_back();
	}

	inline void leave_scope() {
		// Ensure do not pop global scope.
		assert(this->table.size() > 1);
		this->table.pop();
	}

	// Add a new or replace symbol.
	inline IdenAttr& new_identifier(UniversalIdType id, IdenAttr attr) {
		auto [it, _] = this->table.top().emplace(id, attr);
		return it->second;
	}

	// Add a new or replace global symbol.
	inline void new_identifier_global(UniversalIdType id, IdenAttr attr) {
		this->table.bottom().emplace(id, attr);
	}
};

} // namespace scr
