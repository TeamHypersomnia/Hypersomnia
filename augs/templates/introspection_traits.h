#pragma once
#include <type_traits>
#include "augs/templates/predicate_templates.h"

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