#include "../src/core/cplusplus/utilities/unique_string.hpp"

#include <cstddef>
#include <print>

using namespace core;

static constexpr size_t ITERATION = 5;

int main() {
	UniqueStringGenerator generator;

	for (int i = 0; i < ITERATION * 26; i++) {
		std::println("Generated: {}", generator.generate());
	}
}
