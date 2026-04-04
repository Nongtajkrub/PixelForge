#include "const_pool.hpp"

#include <cstddef>

namespace scr {

ConstIndex ConstPool::intern_const(const Const& value) {
	const auto [index, inserted] = this->const_index.intern(value);

	if (inserted) {
		this->pool.push(value);
	}

	return index;
}

} // namespace scr
