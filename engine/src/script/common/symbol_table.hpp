#pragma once

#include "../ast/ast.hpp"
#include "../../core/id/incremental.hpp"
#include "../../core/stack/iterable.hpp"

#include <cstddef>
#include <unordered_map>
#include <utility>
#include <unordered_set>
#include <ranges>

namespace scr {

using namespace core;
 
struct IdenAttr {
	// Symbol data type (Can also be return type).
	ASTNode type;

	explicit IdenAttr(ASTNode type) :
		type(type)
	{ }
};

class SymbolTable {
// Identifier table store information about scopes and additional attr.
using IdenTable =
	std::unordered_map<
		UniversalIdType, std::unordered_map<std::string, IdenAttr>>;
// Type pool store datatypes that exist, types are alway global.
using TypePool = std::unordered_set<std::string>; 

private:
	IdenTable idens;
	TypePool types;
	IterableStack<UniversalIdType> scopes;

public:
	SymbolTable();

	inline void push_scope() {
		this->scopes.push(IncrementalIdGen::generate());
	}

	inline void pop_scope() {
		this->scopes.pop();
	}

	inline bool is_scope_global() {
		return this->scopes.top() == this->scopes.bottom();
	}

	// Add a new or replace symbol.
	inline void set_iden(const std::string& symbol, IdenAttr attr) {
		this->idens[this->scopes.top()].emplace(symbol, attr);
	}

	// Add a new or replace global symbol.
	inline void set_iden_global(const std::string& symbol, IdenAttr attr) {
		this->idens[this->scopes.bottom()].emplace(symbol, attr);
	}

	// Check whether a symbol exist.
	inline bool is_iden_exist(const std::string& symbol) {
		for (const auto& scope : std::views::reverse(this->scopes)) {
			const auto scope_it = idens.find(scope);
			if (scope_it != idens.end() && scope_it->second.contains(symbol)) {
				return true;
			}
		}
		return false;
	}

	inline void new_type(const std::string& symbol) {
		this->types.insert(symbol);
	}

	inline bool is_type_exis(const std::string& symbol) {
		return this->types.contains(symbol);
	}
};

} // namespace scr
