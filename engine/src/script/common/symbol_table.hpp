#pragma once

#include "../ast/ast.hpp"
#include "../../core/id/incremental.hpp"

#include <cstddef>
#include <optional>
#include <unordered_map>
#include <utility>

namespace scr {

using namespace core;
 
struct SymbolAttributes {
	// Symbol data type (Can also be return type).
	std::optional<ASTNode> type;

	explicit SymbolAttributes(ASTNode type) :
		type(type)
	{ }
};

class SymbolTable {
using TableMap =
	std::unordered_map<
		UniversalIdType, std::unordered_map<std::string, SymbolAttributes>>;

private:
	TableMap table;

public:
	// Add a new or replace symbol.
	inline void set(
		UniversalIdType scope,
		const std::string& symbol, SymbolAttributes attrs) {
		this->table[scope].emplace(symbol, attrs);
	}

	// Check whether a symbol exist.
	inline bool is_exist(UniversalIdType scope, const std::string& symbol) {
		const auto& scope_it = this->table.find(scope);

		if (scope_it == this->table.end()) {
			return false;
		}

		const auto& symbol_it = (scope_it->second).find(symbol);

		return symbol_it != (scope_it->second).end();
	}
};

} // namespace scr
