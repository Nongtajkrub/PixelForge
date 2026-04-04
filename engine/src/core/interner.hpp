#pragma once

#include "incremental_id.hpp"

#include <concepts>
#include <cstddef>
#include <functional>
#include <unordered_map>

namespace core {

template <typename T>
concept Hashable = requires(T a) {
	{ std::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
} && std::equality_comparable<T>;

// An ID interner handle ID generation for specfic values, where identical
// values share the same ID.
// T: The type you want to intern to ID.
// I: The ID type you want to use.
template <typename T, typename I>
requires Hashable<T> && IdType<I>
class IdInterner {
private:
	std::unordered_map<T, I> intern_table;

	// The ID generation function default to incremental ID.
	std::function<I()> id_generator;

public:
	IdInterner() = default;
	explicit IdInterner(std::function<I()> id_generator) :
		id_generator(id_generator)
	{ }

	std::pair<I, bool> intern(const T& value) {
		auto [it, inserted] =
			this->intern_table.try_emplace(value, I{});

		if (inserted) {
			it->second = this->id_generator();
		}

		return {it->second, inserted};
	}
};

} // namespace core
