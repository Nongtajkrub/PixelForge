#include <core-cplusplus/types.hpp>

#include <utility>
#include <tuple>

namespace core {

template<typename ...Stages>
class Pipeline {
private:
	static constexpr std::tuple<Stages...> stages;

	template<typename TInput, typename FFirst, typename... FRest>
	static constexpr decltype(auto) nested_call(
		TInput&& input, FFirst&& first, FRest&&... rest) {
		if constexpr (sizeof...(rest) == 0) {
			return first(std::forward<TInput>(input));
		} else {
			return first(
				nested_call(
					std::forward<TInput>(input), std::forward<FRest>(rest)...));
		}
	}

public:
	template<typename TInput>
	static constexpr decltype(auto) execute(TInput&& input) {
		using Sequence = 
			reverse_index_sequence_t<
				std::make_index_sequence<std::tuple_size_v<decltype(stages)>>>;

		return [&input]<size_t... Is>(std::index_sequence<Is...>) -> decltype(auto) {
			nested_call(std::forward<TInput>(input), std::get<Is>(stages)...);
		}(Sequence());
	}
};

}
