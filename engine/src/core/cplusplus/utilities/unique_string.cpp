#include "unique_string.hpp"

#include <cstddef>

namespace core {

static constexpr size_t SMALL_STR_LEN = 16;

std::string UniqueStringGenerator::generate() {
	size_t n = this->counter++;
	std::string buffer;

	buffer.append(prefix);

	// Convert to base-26 (a–z)
	do {
		char c = 'a' + (n % 26);
		buffer.push_back(c);
		n /= 26;
	} while (n > 0);

	return buffer;
}

} // namespace core
