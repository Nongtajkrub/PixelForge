#pragma once

#include "../core/cplusplus/utilities/id_interner.hpp"
#include "../core/cplusplus/utilities/variant.hpp"
#include "../core/cplusplus/container/pool.hpp"
#include "../core/cplusplus/io/log.hpp"
#include "../core/cplusplus/types.hpp"
#include "specs.h"
#include "token.hpp"

#include <cassert>
#include <cstddef>
#include <functional>
#include <string_view>

namespace scr {

using namespace core;
using ConstIndex = word_t;
using StringIndex = word_t;

struct Const {
	Variant<i32, f32, bool, std::string> data;

	explicit Const(const Token& literal);

	TokenKind get_type() const;

	bool operator==(const Const& other) const {
		if (this->data.is<i32>()) {
			return this->data.get<i32>() == other.data.get<i32>();
		} else if (this->data.is<f32>()) {
			return this->data.get<f32>() == other.data.get<f32>();
		} else if (this->data.is<bool>()) {
			return this->data.get<bool>() == other.data.get<bool>();
		} else if (this->data.is<std::string>()) {
			return this->data.get<std::string>() == other.data.get<std::string>();
		}

		LOG_ERR("Unimplemented eq operator Const.");
		exit(1);
	}
};

} // namespace scr 

template <>
struct std::hash<scr::Const> {
	size_t operator()(const scr::Const& value) const {
		size_t h = 0;

		if (value.data.is<i32>()) {
			h = std::hash<i32>{ }(value.data.get<i32>());
		} else if (value.data.is<f32>()) {
			h = std::hash<f32>{ }(value.data.get<f32>());
		} else if (value.data.is<bool>()) {
			h = std::hash<bool>{ }(value.data.get<bool>());
		} else if (value.data.is<std::string>()) {
			h = std::hash<std::string>{ }(value.data.get<std::string>());
		} else {
			LOG_ERR("Unimplemented hash Const.");
			exit(1);
		}

		return (h << 1);;
	}
};

namespace scr {

class ConstPool {
private:
	Pool<Const> pool;
	// Assigns a unique index to each const; identical share same index.
	IdInterner<Const, ConstIndex> const_index; 

public:
	ConstPool() :
		const_index([this]() -> ConstIndex {
			return this->pool.size();
		})
	{ }

	// Push a const into pool if it does not already exist and return const index.
	ConstIndex intern_const(const Const& value);

	std::vector<u8> serialize();

	inline const Const& get(ConstIndex index) const {
		return this->pool[index];
	}
};

} // namespace scr
