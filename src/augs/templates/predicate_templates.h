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

template <
	template <class...> class LogicalOp,
	template <class...> class UnaryPredicate
>
struct make_variadic_predicate {
	template <class... TypeArguments>
	struct type
		: LogicalOp<UnaryPredicate<TypeArguments>...>
	{
	};
};

template <
	template <class...> class LogicalOp,
	template <class...> class UnaryPredicate
>
using make_variadic_predicate_t = typename make_variadic_predicate<LogicalOp, UnaryPredicate>::type;