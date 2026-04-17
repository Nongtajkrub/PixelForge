#pragma once

#include "../core/cplusplus/types.hpp"
#include "../core/cplusplus/container/pool.hpp"
#include "../core/cplusplus/utilities/id_interner.hpp"
#include "../core/cplusplus/io/log.hpp"
#include "token.hpp"

#include <cassert>
#include <cstddef>
#include <functional>
#include <string_view>

namespace scr {

using namespace core;
using ConstIndex = u16;
using StringIndex = u16;

struct Const {
	// Must be value type.
	const TokenKind type;

	union {
		i32 int_const;
		f32 float_const;
		bool bool_const;
		StringIndex str_index;
	} data;	

	explicit Const(const Token& literal);

	bool operator==(const Const& other) const {
		if (this->type != other.type) {
			return false;
		}

		switch (this->type) {
		case TokenKind::INT_T: 
			return this->data.int_const == other.data.int_const;
		case TokenKind::FLOAT_T: 
			return this->data.float_const == other.data.float_const;
		case TokenKind::BOOL_T: 
			return this->data.bool_const == other.data.bool_const;
		case TokenKind::STRING_T: 
			return this->data.str_index == other.data.str_index;
		default:
			LOG_ERR("Type for constants can only be value type");
			exit(1);
		}
	}
};

} // namespace scr 

template <>
struct std::hash<scr::Const> {
	size_t operator()(const scr::Const& value) const {
		size_t h1 = std::hash<u8>{ }(static_cast<u8>(value.type));

		size_t h2;
		switch (value.type) {
		case scr::TokenKind::INT_T: 
			h2 = std::hash<i32>{ }(value.data.int_const);
			break;
		case scr::TokenKind::FLOAT_T: 
			h2 = std::hash<f32>{ }(value.data.float_const);
			break;
		case scr::TokenKind::BOOL_T: 
			h2 = std::hash<bool>{ }(value.data.bool_const);
			break;
		case scr::TokenKind::STRING_T: 
			h2 = std::hash<scr::ConstIndex>{ }(value.data.str_index);
			break;
		default:
			LOG_ERR("Type for constants can only be value type");
			exit(1);
		}

		return h1 ^ (h2 << 1);;
	}
};

namespace scr {

class ConstPool {
private:
	Pool<Const> pool;
	// Assigns a unique index to each const; identical share same index.
	IdInterner<Const, ConstIndex> const_index; 

	Pool<std::string> str_pool;

public:
	ConstPool() :
		const_index([this]() -> ConstIndex {
			return this->pool.size();
		})
	{ }

	// Push a const into pool if it does not already exist and return const index.
	ConstIndex intern_const(const Const& value);

	inline const Const& get(ConstIndex index) const {
		return this->pool[index];
	}
};

} // namespace scr
