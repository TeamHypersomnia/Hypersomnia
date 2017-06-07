#pragma once
#include <type_traits>

template <
	template <class...> class TypePredicate
>
struct apply_negation {
	template <class... PredicateArguments>
	struct type
		: std::negation<TypePredicate<PredicateArguments...>> {
	};
};

template <
	template <class...> class TypePredicate
>
using apply_negation_t = typename apply_negation<TypePredicate>::type;