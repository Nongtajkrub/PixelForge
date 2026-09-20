#include <core-cplusplus/types.hpp>

#include <type_traits>
#include <optional>
#include <utility>
#include <tuple>

namespace core {

template<typename T>
using PipelineOut = std::optional<T>;

constexpr auto plterminate = std::nullopt;

template<typename ...Stages>
class Pipeline {
private:
	static constexpr std::tuple<Stages...> stages;

	template<typename TInput, typename TCtx, typename FFirst, typename... FRest>
	static constexpr decltype(auto) nested_call(
		TInput&& input, TCtx&& ctx, FFirst&& first, FRest&&... rest) {
		if constexpr (sizeof...(rest) == 0) {
			return first(std::forward<TInput>(input), std::forward<TCtx>(ctx));
		} else {
			const auto prev = nested_call(
				std::forward<TInput>(input),
				std::forward<TCtx>(ctx), std::forward<FRest>(rest)...);

			using RetType = decltype(first(*prev, std::forward<TCtx>(ctx)));
			static_assert(is_optional_v<std::remove_cvref_t<RetType>>);

			if (!prev) {
				return (RetType) { std::nullopt };
			}

			return first(*prev, std::forward<TCtx>(ctx));
		}
	}

public:
	template<typename TInput, typename TCtx>
	static constexpr decltype(auto) execute(TInput&& input, TCtx&& ctx) {
		using Sequence = 
			reverse_index_sequence_t<
				std::make_index_sequence<std::tuple_size_v<decltype(stages)>>>;

		return [&input, &ctx]<size_t... Is>(
			std::index_sequence<Is...>) -> decltype(auto) {
			return nested_call(
				std::forward<TInput>(input),
				std::forward<TCtx>(ctx), std::get<Is>(stages)...);
		}(Sequence());
	}
};

}
