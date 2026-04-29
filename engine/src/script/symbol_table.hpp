#pragma once

#include "../core/cplusplus/utilities/incremental_id.hpp"
#include "../core/cplusplus/container/cursor_stack.hpp"
#include "../core/cplusplus/utilities/free_list_id.hpp"
#include "../core/cplusplus/utilities/id_interner.hpp"
#include "../core/cplusplus/utilities/variant.hpp"
#include "../core/cplusplus/io/log.hpp"
#include "specs.h"

#include <unordered_map>
#include <type_traits>
#include <optional>
#include <concepts>
#include <cassert>
#include <cstddef>
#include <utility>
#include <stack>

namespace scr {

using namespace core;
using IdentifierId = u32;
using VariableSlot = word_t;
using ArgStackIndex = word_t;

// Forward declaration.
struct TypeAttr;

struct VarAttr {
	VariableSlot slot;
	TypeAttr* type;
};

struct ArgAttr{
	ArgStackIndex index;
	TypeAttr* type;
};

enum class TypeId {
	VOID,
	INT,
	FLOAT,
	BOOL,
	STR,
	SPRITE
};

struct TypeAttr {
	TypeId id;
	bool is_value;

	// Contains function ID for each routines methods.
	struct {
		std::optional<word_t> add = std::nullopt;
		std::optional<word_t> minus = std::nullopt;
		std::optional<word_t> eq = std::nullopt;
		std::optional<word_t> size = std::nullopt;
	} routines;

	std::optional<std::vector<VarAttr>> properties = std::nullopt;
};

struct FuncAttr {
	std::vector<TypeAttr*> args_types;
	TypeAttr* type;

	inline size_t arg_n() const {
		return this->args_types.size();
	}
};

template <typename T>
concept AttrType = std::same_as<T, TypeAttr> 
	|| std::same_as<T, VarAttr> 
	|| std::same_as<T, ArgAttr> || std::same_as<T, FuncAttr>;

class IdenAttr {
public:
	IdenAttr() = default;

	// Constructor for identifiers with a type.
	template <typename T>
	requires (AttrType<std::remove_cvref_t<T>> 
		&& !std::is_same_v<std::remove_cvref_t<T>, TypeAttr>)
	IdenAttr(TypeAttr* type, T&& attr) :
		data(std::forward<T>(attr))
	{ 
		using U = std::remove_cvref_t<T>;
		
		if constexpr (std::same_as<U, VarAttr>) {
			get_data<VarAttr>().type = type;
		} else if constexpr (std::same_as<U, ArgAttr>) {
			get_data<ArgAttr>().type = type;
		} else if constexpr (std::same_as<U, FuncAttr>) {
			get_data<FuncAttr>().type = type;
		} else {
			BUG("Assigning type for identifier not implemented.");
			exit(1);
		}
	}

	// Constructor for identifier with no type.
	template <typename T>
	requires std::is_same_v<std::remove_cvref_t<T>, TypeAttr>
	IdenAttr(T&& attr) :
		data(std::forward<T>(attr))
	{ }

	inline TypeAttr* get_type() {
		if (this->data.is<VarAttr>()) {
			return this->data.get<VarAttr>().type;
		} else if (this->data.is<ArgAttr>()) {
			return this->data.get<ArgAttr>().type;
		} else if (this->data.is<FuncAttr>()) {
			return this->data.get<FuncAttr>().type;
		} else if (this->data.is<TypeAttr>()) {
			return &this->data.get<TypeAttr>();
		} else {
			BUG("Unimplemented get_type for IdenAttr.");
			exit(1);
		}
	}

	template <typename T>
	requires AttrType<T>
	inline bool kind_is() const {
		return this->data.is<T>();
	}

	template <typename T>
	requires AttrType<T>
	inline T& get_data() {
		return this->data.get<T>();
	}

	template <typename T>
	requires AttrType<T>
	inline const T& get_data() const {
		return this->data.get<T>();
	}

	bool in_scope = true;

private:
	Variant<VarAttr, ArgAttr, FuncAttr, TypeAttr> data;
};

enum class ScopeKind {
	GLOBAL,
	IF,
	FUNC,
	LOOP,
};

struct Scope {
	ScopeKind kind;

	// Owner of the scope if exist (Often a function).
	IdenAttr* owner;
	
	// Owner of the scope if exist (Usually function).
	// IdenAttr store in a stack to support identifier shadowing.
	std::unordered_map<IdentifierId, std::stack<IdenAttr>> table;

	Scope() = default;
	Scope(ScopeKind kind, IdenAttr* owner = nullptr) : 
		kind(kind), owner(owner), table() 
	{ }

};
 
class SymbolTable {
public:
	struct {
		TypeAttr* ty_void;
		TypeAttr* ty_int;
		TypeAttr* ty_float;
		TypeAttr* ty_bool;
		TypeAttr* ty_str;
		TypeAttr* ty_sprite;
	} types;

public:
	SymbolTable();

	// Check whether a symbol exist.
	bool contains(IdentifierId id);

	// Check whether symbol exist in the current scope only.
	bool contains_in_scope(IdentifierId id);

	bool in_scope(ScopeKind kind);

	IdenAttr* get_scope_owner();

	// Look up a symbol and return the reference to its attr.
	IdenAttr* lookup(IdentifierId id);

public:
	inline IdenAttr* lookup_global(IdentifierId id) {
		return lookup(id, this->scopes.bottom());
	}

	// Intern an identifier to ID.
	inline IdentifierId intern(const std::string& iden) {
		auto [id, _] = this->iden_interner.intern(iden);
		return id;
	}

	inline void enter_scope(ScopeKind kind, IdenAttr* owner = nullptr) {
		auto [scope, inserted] = this->scopes.try_emplace_back(kind, owner);

		// If no new scope was inserted (cursor reused an existing slot),
		// construct the scope manually.
		if (!inserted) {
			scope.kind = kind;
			scope.owner = owner;
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
				if (attr.kind_is<VarAttr>()) {
					this->var_slot_generator.free(attr.get_data<VarAttr>().slot);
				}
			}
		}
		
		this->scopes.rewind();
	}

	inline ScopeKind this_scope() {
		return this->scopes.top().kind;
	}

	// Add a new or replace symbol.
	inline IdenAttr* new_identifier(IdentifierId id, IdenAttr attr) {
		return new_identifier(id, attr, this->scopes.top());
	}

	// Add a new or replace global symbol.
	inline IdenAttr* new_identifier_global(IdentifierId id, IdenAttr attr) {
		return new_identifier(id, attr, this->scopes.bottom());
	}

private:
	// Identifier table stored in a cursor stack to manage scopes while
	// preserving identifier attributes, ensuring references remain valid.	
	CursorStack<Scope> scopes;

	// Assigns a unique slot to each identifier, identical share same slot.
	IdInterner<std::string, IdentifierId> iden_interner; 

	// Generate ID for identifiers.
	IncrementalIdGen<IdentifierId> iden_id_generator = 
		IncrementalIdGen<IdentifierId>();

	// Generate slot for variable identifiers.
	FreeListIdGen<VariableSlot> var_slot_generator =
		FreeListIdGen<VariableSlot>();

	// Generate slot for function arguments.
	IncrementalIdGen<ArgStackIndex> stack_index_generator =
		IncrementalIdGen<ArgStackIndex>();

private:
	inline IdenAttr* new_identifier(
		IdentifierId id, IdenAttr attr, Scope& scope) {
		if (attr.kind_is<VarAttr>()) {
			attr.get_data<VarAttr>().slot = this->var_slot_generator.generate();
		} else if (attr.kind_is<ArgAttr>()) {
			attr.get_data<ArgAttr>().index =
				this->stack_index_generator.generate();
		}

		auto [it, _] = scope.table.try_emplace(id);
		it->second.emplace(std::move(attr));
		return &it->second.top();
	}

	// Look up a symbol and return the reference to its attr.
	IdenAttr* lookup(IdentifierId id, Scope& scope);
};

} // namespace scr
