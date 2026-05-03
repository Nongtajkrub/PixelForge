#include "fscript_const_pool.hpp"

#include "../core/cplusplus/io/byte_io.hpp"
#include "fscript_token.hpp"
#include "fscript_specs.h"

#include <cassert>
#include <cstddef>
#include <vector>

namespace scr {

Const::Const(const Token& literal) {
	switch (literal.kind) {
	case TokenKind::INTEGER_LIT:
		this->data = (i32)std::stoi(*(literal.lexeme));
		break;
	case TokenKind::FLOAT_LIT:
		this->data = (f32)std::stof(*(literal.lexeme));
		break;
	default:
		BUG("Token can't be converrt to Const.");
		exit(1);
	}
}

ConstIndex ConstPool::intern(const Const& value) {
	const auto [index, inserted] = this->const_index.intern(value);

	if (inserted) {
		this->pool.push(value);
	}

	return index;
}

void ConstPool::serialize(std::vector<u8>& buf) const {
	std::vector<u8> entry_buf;
	entry_buf.reserve(1024);

	for (const auto& entry : this->pool) {
		if (entry.data.is<i32>()) {
			push_bytes<word_t>(entry_buf, sizeof(i32));
			push_bytes<i32>(entry_buf, entry.data.get<i32>());
		} else if (entry.data.is<f32>()) {
			push_bytes<word_t>(entry_buf, sizeof(f32));
			push_bytes<f32>(entry_buf, entry.data.get<f32>());
		} else {
			BUG("Unimplemented get_type Const.");
			exit(1);
		}
	}

	// Concat size information, address buffer and value buffer together
	buf.reserve(WORD_SIZE + entry_buf.size());
	push_bytes<word_t>(buf, entry_buf.size());
	buf.insert(buf.end(), entry_buf.begin(), entry_buf.end());
}

} // namespace scr
