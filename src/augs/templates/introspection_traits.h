#pragma once
#include <type_traits>
#include "augs/templates/predicate_templates.h"

namespace augs {
	struct introspection_access;
}

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
		augs::introspection_access::introspect_body(
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

template <class T, class = void>
struct has_enum_to_string : std::false_type {};

template <class T>
struct has_enum_to_string<T, decltype(augs::enum_to_string(T()), void())> 
	: std::true_type 
{};

template <class T>
constexpr bool has_enum_to_string_v = has_enum_to_string<T>::value;

template <class T, class = void>
struct has_for_each_enum : std::false_type {};

template <class T>
struct has_for_each_enum<T, decltype(augs::enum_to_args_impl(T(), true_returner()), void())>
	: std::true_type
{};

template <class T>
constexpr bool has_for_each_enum_v = has_for_each_enum<T>::value;

template <class T>
struct is_introspective_leaf : 
	std::bool_constant<
		std::is_enum_v<T>
		|| std::is_arithmetic_v<T>
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