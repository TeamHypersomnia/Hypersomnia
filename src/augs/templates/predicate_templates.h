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

template <
	template <class...> class TypePredicate,
	class... BoundTypes
>
struct bind_types {
	template <class... PredicateArguments>
	struct type
		: TypePredicate<BoundTypes..., PredicateArguments...> {
	};
};

template <
	template <class...> class TypePredicate,
	class... BoundTypes
>
using bind_types_t = typename bind_types<TypePredicate, BoundTypes...>::type;

template <
	template <class...> class TypePredicate,
	class... BoundTypes
>
struct bind_types_right {
	template <class... PredicateArguments>
	struct type
		: TypePredicate<PredicateArguments..., BoundTypes...> {
	};
};

template <
	template <class...> class TypePredicate,
	class... BoundTypes
>
using bind_types_right_t = typename bind_types_right<TypePredicate, BoundTypes...>::type;