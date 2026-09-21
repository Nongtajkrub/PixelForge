#include "fscript_const_pool.hpp"

#include <core-cplusplus/io/byte_io.hpp>

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

std::vector<u8> ConstPool::serialize() const {
	using HoleToken = core::BytesBufferWriter::HoleToken;

	std::vector<u8> buf;
	auto io = core::BytesBufferWriter(buf);

	HoleToken size_hole = io.hole<word_t>();

	for (const auto& entry : this->pool) {
		if (entry.data.is<i32>()) {
			io.write<word_t>(sizeof(i32));
			io.write<i32>(entry.data.get<i32>());
		} else if (entry.data.is<f32>()) {
			io.write<word_t>(sizeof(f32));
			io.write<i32>(entry.data.get<f32>());
		} else {
			BUG("Unimplemented get_type Const.");
			exit(1);
		}
	}

	// Concat size information, address buffer and value buffer together
	io.fill<word_t>(buf.size(), size_hole);

	return buf;
}

} // namespace scr
