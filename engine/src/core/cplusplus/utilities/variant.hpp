#pragma once

#include <variant>
#include <concepts>
#include <type_traits>

namespace core {

template <typename ...T>
class Variant {
private:
	std::variant<T...> data;

public:
	Variant() = default;
	template <typename U>
	requires (std::same_as<std::decay_t<U>, T> || ...)
	Variant(U&& data) :
		data(std::forward<U>(data))
	{}

	template <typename U>
	requires (std::same_as<std::decay_t<U>, T> || ...)
	bool is() const {
		return std::holds_alternative<U>(this->data);
	}

	template <typename U>
	requires (std::same_as<std::decay_t<U>, T> || ...)
	U& get() {
		return std::get<U>(this->data);
	}

	template <typename U>
	requires (std::same_as<std::decay_t<U>, T> || ...)
	const U& get() const {
		return std::get<U>(this->data);
	}
};

} // namespace core
