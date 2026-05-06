#pragma once

#include "../core/cplusplus/utilities/incremental_id.hpp"
#include "../core/cplusplus/container/cursor_stack.hpp"
#include "../core/cplusplus/utilities/id_interner.hpp"
#include "../core/cplusplus/utilities/variant.hpp"
#include "../core/c/io/log.h"
#include "fscript_specs.h"

#include <unordered_map>
#include <type_traits>
#include <algorithm>
#include <concepts>
#include <cassert>
#include <cstddef>
#include <utility>
#include <stack>

namespace scr {

using namespace core;
// A unique ID for each identifier, identical share same ID.
using IdentifierId = u32;

// The offset between where the property is store and the start of property list.
using PropOffset = word_t;

// The offset between where the variable is store and the start of the stack.
using StackOffset = word_t;

// Forward declaration.
struct TypeAttr;

// Wont appear in symbol table, only use in TypeAttr.
struct PropAttr {
	// Properties wont be insert into the table so they store their own ID.
	IdentifierId id;
	TypeAttr* type;

	PropAttr(IdentifierId id, TypeAttr* type) : 
		id(id), type(type)
	{ }
};

struct TypeAttr {
	// Store their own ID so comparing type with eachother is easier.
	IdentifierId id;
	bool is_value;

	std::vector<PropAttr> properties;

	std::vector<TypeAttr*> get_prop_types() const;

	inline PropAttr* get_prop(IdentifierId id) {
		const auto it =
			std::find_if(
				this->properties.begin(),
				this->properties.end(),
				[id](const PropAttr& prop) { return prop.id == id; });
		return (it == this->properties.end()) ? nullptr : &(*it);
	}

	inline std::pair<PropAttr*, size_t> get_prop_offset(IdentifierId id) {
		const auto it =
			std::find_if(
				this->properties.begin(),
				this->properties.end(),
				[id](const PropAttr& prop) { return prop.id == id; });

		if (it == this->properties.end()) return {nullptr, 0};
		return {&(*it), it - this->properties.begin()};
	}

	inline bool has_prop(IdentifierId id) {
		return get_prop(id) != nullptr;
	}

	inline void extend(TypeAttr* type) {
		this->properties.insert(
			this->properties.end(),
			type->properties.begin(), type->properties.end());
	}
};

struct VarAttr{
	StackOffset offset;
	TypeAttr* type;
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
	|| std::same_as<T, VarAttr> || std::same_as<T, FuncAttr>;

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
	IdentifierId id;

private:
	Variant<VarAttr,  FuncAttr, TypeAttr> data;
};

enum class ScopeKind {
	GLOBAL,
	IF,
	FUNC,
	UPDATE,
	INTERFACE,
	LOOP,
};

struct Scope {
	ScopeKind kind;

	// Owner of the scope if exist (Often a function).
	IdenAttr* owner;

	// For generating the stack offset of each variable, reset when leave scope.
	IncrementalIdGen<StackOffset> stack_offset_gen = 
		IncrementalIdGen<StackOffset>(0);
	
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
		TypeAttr* ty_sprite;
	} types;

public:
	SymbolTable();

	void enter_scope(ScopeKind kind, IdenAttr* owner = nullptr);
	void leave_scope();

	// Check whether a symbol exist.
	bool contains(IdentifierId id);

	// Check whether symbol exist in the current scope only.
	bool contains_in_scope(IdentifierId id);

	bool in_scope(ScopeKind kind);

	IdenAttr* get_scope_owner();

	// Look up a symbol and return the reference to its attr.
	IdenAttr* lookup(IdentifierId id);

	// Intern an identifier to ID.
	inline IdentifierId intern(const std::string& iden) {
		auto [id, _] = this->iden_interner.intern(iden);
		return id;
	}

	inline IdenAttr* lookup_global(IdentifierId id) {
		return lookup(id, this->scopes.bottom());
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

	// Generate ID for identifiers.
	IncrementalIdGen<IdentifierId> iden_id_generator = 
		IncrementalIdGen<IdentifierId>();

	// Assigns a unique ID to each identifier, identical share same ID.
	IdInterner<std::string, IdentifierId> iden_interner =
		IdInterner<std::string, IdentifierId>([this]() -> IdentifierId {
			return this->iden_id_generator.generate(); 
		});

private:
	IdenAttr* new_identifier(
		IdentifierId id, IdenAttr attr, Scope& scope);

	// Look up a symbol and return the reference to its attr.
	IdenAttr* lookup(IdentifierId id, Scope& scope);

	TypeAttr construct_void_attr();
	TypeAttr construct_int_attr();
	TypeAttr construct_float_attr();
	TypeAttr construct_bool_attr();
	TypeAttr construct_sprite_attr();
};

} // namespace scr
