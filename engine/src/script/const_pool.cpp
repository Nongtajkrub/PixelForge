#include "const_pool.hpp"
#include "token.hpp"

#include <cassert>
#include <cstddef>
#include <utility>

namespace scr {

Const::Const(const Token& literal) :
	type(token_to_type(literal.kind))
{
	assert(token_is_value_type(this->type));

	switch (this->type) {
	case TokenKind::INT_T:
		this->data.int_const = std::stoi(*(literal.lexeme));
		break;
	case TokenKind::FLOAT_T:
		this->data.float_const = std::stof(*(literal.lexeme));
		break;
	case TokenKind::BOOL_T:
		this->data.bool_const = token_to_boolean(literal.kind);
		break;
	case TokenKind::STRING_T:
		this->data.str_index = 0;
		break;
	default:
		LOG_ERR("Literals should only be value type.");
		std::unreachable();
	}
}

ConstIndex ConstPool::intern_const(const Const& value) {
	const auto [index, inserted] = this->const_index.intern(value);

	if (inserted) {
		this->pool.push(value);
	}

	return index;
}

} // namespace scr
