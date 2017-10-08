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
	template <class...> class TypePredicate,
	class... BoundTypes
>
struct bind_types {
	template <class... PredicateArguments>
	struct type
		: TypePredicate<BoundTypes..., PredicateArguments...> {
	};
};