#pragma once

#include <cstdint>
#include <optional>
#include <functional>
#include <type_traits>

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using f32 = float;

namespace core {

template <typename T>
using Ref = std::reference_wrapper<T>;

template<typename T>
struct reverse_index_sequence;

template<size_t... Is>
struct reverse_index_sequence<std::index_sequence<Is...>> {
	using type = std::index_sequence<((sizeof...(Is) - 1) - Is)...>;
};

template<typename T>
using reverse_index_sequence_t = reverse_index_sequence<T>::type;

template<typename T>
struct is_optional : std::false_type {};

template<typename T>
struct is_optional<std::optional<T>> : std::true_type {};

template<typename T>
constexpr auto is_optional_v = is_optional<T>::value;

}
