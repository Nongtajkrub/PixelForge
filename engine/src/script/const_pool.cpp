#include "const_pool.hpp"

#include "../core/cplusplus/io/byte_io.hpp"
#include "specs.h"
#include "token.hpp"

#include <cassert>
#include <cstddef>
#include <print>
#include <vector>

namespace scr {

Const::Const(const Token& literal) {
	const auto type = token_to_type(literal.kind);
	assert(token_is_value_type(type));

	switch (type) {
	case TokenKind::INT_T:
		this->data = (i32)std::stoi(*(literal.lexeme));
		break;
	case TokenKind::FLOAT_T:
		this->data = (f32)std::stof(*(literal.lexeme));
		break;
	case TokenKind::BOOL_T:
		this->data = (bool)token_to_boolean(literal.kind);
		break;
	case TokenKind::STRING_T:
		this->data = (std::string)*literal.lexeme;
		break;
	default:
		LOG_ERR("Literals should only be value type.");
		exit(1);
	}
}

TokenKind Const::get_type() const {
	if (this->data.is<i32>()) {
		return TokenKind::INT_T;
	} else if (this->data.is<f32>()) {
		return TokenKind::FLOAT_T;
	} else if (this->data.is<bool>()) {
		return TokenKind::BOOL_T;
	} else if (this->data.is<std::string>()) {
		return TokenKind::STRING_T;
	} else {
		LOG_ERR("Unimplemented get_type Const.");
		exit(1);
	}
}

ConstIndex ConstPool::intern_const(const Const& value) {
	const auto [index, inserted] = this->const_index.intern(value);

	if (inserted) {
		this->pool.push(value);
	}

	return index;
}

std::vector<u8> ConstPool::serialize() {
	std::vector<u8> addr_buff;
	addr_buff.reserve(256);

	std::vector<u8> entry_buff;
	entry_buff.reserve(1024);

	for (const auto& entry : this->pool) {
		push_bytes<word_t>(addr_buff, entry_buff.size());

		if (entry.data.is<i32>()) {
			push_bytes<word_t>(entry_buff, sizeof(i32));
			push_bytes<i32>(entry_buff, entry.data.get<i32>());
		} else if (entry.data.is<f32>()) {
			push_bytes<f32>(entry_buff, entry.data.get<f32>());
			push_bytes<word_t>(entry_buff, sizeof(f32));
			push_bytes<i32>(entry_buff, entry.data.get<i32>());
		} else if (entry.data.is<bool>()) {
			push_bytes<word_t>(entry_buff, sizeof(bool));
			push_bytes<bool>(entry_buff, entry.data.get<bool>());
		} else if (entry.data.is<std::string>()) {
			const auto str = entry.data.get<std::string>();
			push_bytes<word_t>(entry_buff, str.size());
			push_bytes(entry_buff, str.data(), str.size());
		} else {
			LOG_ERR("Unimplemented get_type Const.");
			exit(1);
		}
	}

	// Concat size information, address buffer and value buffer together
	std::vector<u8> buff;
	buff.reserve((sizeof(word_t) * 2) + addr_buff.size() + entry_buff.size());

	std::println("data size: {}", addr_buff.size() + entry_buff.size());

	push_bytes<word_t>(buff, addr_buff.size());
	buff.insert(buff.end(), addr_buff.begin(), addr_buff.end());
	push_bytes<word_t>(buff, entry_buff.size());
	buff.insert(buff.end(), entry_buff.begin(), entry_buff.end());

	return buff;
}

} // namespace scr
