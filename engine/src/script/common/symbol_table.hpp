#pragma once

#include "../../core/stack/iterable.hpp"
#include "../../core/id/incremental.hpp"

#include <cassert>
#include <cstddef>
#include <unordered_map>
#include <unordered_set>
#include <ranges>

namespace scr {

using namespace core;
 
class SymbolTable {
private:
	// Assigns a unique ID to each string; identical strings share the same ID.
	std::unordered_map<std::string, UniversalIdType> iden_interner; 

	// Identifier table store in a stack to manage scope.
	IterableStack<std::unordered_set<UniversalIdType>> scopes;

public:
	SymbolTable();

	inline void enter_scope() {
		this->scopes.emplace_back();
	}

	inline void leave_scope() {
		// Ensure do not pop global scope.
		assert(this->scopes.size() > 1);
		this->scopes.pop();
	}

	inline UniversalIdType intern_iden(const std::string& iden) {
		auto [it, inserted] = iden_interner.try_emplace(iden, UniversalIdType{});

		if (inserted) {
			it->second = IncrementalIdGen::generate();
		}

		return it->second;
	}

	// Add a new or replace symbol.
	inline void new_identifier(UniversalIdType id) {
		this->scopes.top().insert(id);
	}

	// Add a new or replace global symbol.
	inline void new_identifier_global(UniversalIdType id) {
		this->scopes.bottom().insert(id);
	}

	// Check whether a symbol exist.
	inline bool is_iden_exist(UniversalIdType id) {
		for (const auto& scope : std::views::reverse(this->scopes)) {
			if (scope.contains(id)) {
				return true;
			}
		}
		return false;
	}
};

} // namespace scr
