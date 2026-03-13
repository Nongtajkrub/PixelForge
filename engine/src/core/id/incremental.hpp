#pragma once

#include "../../global.hpp"

namespace core {

using UniversalIdType = u32;

class IncrementalIdGen {
private:
	static inline UniversalIdType prev = 0;

public:
	static inline UniversalIdType generate() {
		return IncrementalIdGen::prev++;
	}
};

} // namespace core
