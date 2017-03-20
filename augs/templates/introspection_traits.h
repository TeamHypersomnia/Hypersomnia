#pragma once
#include <type_traits>

struct true_returner {
	template <class... Types>
	bool operator()(Types...) const {
		return true;
	}
};

template <class T, class = void>
struct has_introspect : std::false_type {};

template <class T>
struct has_introspect<
	T,
	decltype(
		augs::introspect_body(
			static_cast<T*>(nullptr),
			true_returner(),
			std::declval<T>()
		),
		void()
	)
> : std::true_type {
};

template <class T>
constexpr bool has_introspect_v = has_introspect<T>::value;

namespace std {
	template <size_t I>
	class bitset;
}

namespace augs {
	template <class T>
	class enum_bitset;
}

template <class T>
struct is_bitset_detail : std::false_type {

};

template <class T>
struct is_bitset_detail<augs::enum_bitset<T>> : std::true_type {

};

template <size_t I>
struct is_bitset_detail<std::bitset<I>> : std::true_type {

};

template <class T>
struct is_bitset : is_bitset_detail<std::remove_cv_t<T>> {

};

template <class T>
constexpr bool is_bitset_v = is_bitset<T>::value;

template <class T>
struct is_introspective_leaf : 
	std::bool_constant<
		std::is_enum_v<T>
		|| std::is_arithmetic_v<T>
		|| is_bitset_v<T>
	> 
{
};

template <class T>
constexpr bool is_introspective_leaf_v = is_introspective_leaf<T>::value;

template <class StreamType, class T, class = void>
struct can_stream_left : std::false_type {

};

template <class StreamType, class T>
struct can_stream_left<StreamType, T, decltype(std::declval<StreamType&>() << std::declval<T>(), void())> : std::true_type {

};

template <class StreamType, class T, class = void>
struct can_stream_right : std::false_type {

};

template <class StreamType, class T>
struct can_stream_right<StreamType, T, decltype(std::declval<StreamType&>() >> std::declval<T&>(), void())> : std::true_type {

};

template <class StreamType, class T>
constexpr bool can_stream_left_v = can_stream_left<StreamType, T>::value;

template <class StreamType, class T>
constexpr bool can_stream_right_v = can_stream_right<StreamType, T>::value;

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

template <class... T>
using do_not_recurse = false_predicate<T...>;

template <class... T>
using always_recurse = true_predicate<T...>;

template <class... T>
using have_introspects = make_variadic_predicate<
	std::conjunction,
	has_introspect
>::type<T...>;

template <class... T>
using at_least_one_is_not_introspective_leaf = make_variadic_predicate<
	std::disjunction,
	apply_negation_t<is_introspective_leaf>
>::type<T...>;

constexpr bool stop_recursion_if_valid = true;

struct no_prologue {
	template <class... Args>
	void operator()(Args&&...) {

	}
};

struct no_epilogue {
	template <class... Args>
	void operator()(Args&&...) {

	}
};