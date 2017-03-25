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
	template <class> class ArgumentTransform
>
struct apply_to_arguments {
	template <class... PredicateArguments>
	struct type
		: TypePredicate<ArgumentTransform<PredicateArguments>...> {
	};
};

template <
	template <class...> class TypePredicate,
	template <class> class ArgumentTransform
>
using apply_to_arguments_t = typename apply_to_arguments<TypePredicate, ArgumentTransform>::type;

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
	template <class...> class... UnaryPredicates
>
struct concat_unary {
	template <class... TypeArguments>
	struct type
		: LogicalOp<UnaryPredicates<TypeArguments>...>
	{
	};
};

template <
	template <class...> class LogicalOp,
	template <class...> class... UnaryPredicates
>
using concat_unary_t = typename concat_unary<LogicalOp, UnaryPredicates...>::type;

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

template <
	template <class...> class LogicalOp,
	template <class...> class... Predicates
>
struct of_predicates {
	template <class T>
	struct type
		: LogicalOp<Predicates<T>...>
	{
	};
};

template <
	template <class...> class LogicalOp,
	template <class...> class... Predicates
>
using of_predicates_t = typename of_predicates<LogicalOp, Predicates...>::type;

template <class...>
struct true_predicate : std::true_type {

};

template <class...>
struct false_predicate : std::false_type {

};

template <bool flag>
struct bool_predicate : std::false_type {
	template <class...>
	struct type : std::bool_constant<flag> {

	};
};

template <bool flag>
using bool_predicate_t = typename bool_predicate<flag>::type;