#pragma once

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

	template <class... Types>
	class trivial_variant;
}

template <class T, class = void>
struct is_trivial_variant : std::false_type {

};

template <class... Types>
struct is_trivial_variant<augs::trivial_variant<Types...>> : std::true_type {

};

template <template <typename...> class C, typename...Ts>
std::true_type is_base_of_template_impl(const C<Ts...>*);

template <template <typename...> class C>
std::false_type is_base_of_template_impl(...);

template <typename T, template <typename...> class C>
using is_base_of_template = decltype(is_base_of_template_impl<C>(std::declval<T*>()));

template <class T>
struct is_base_of_trivial_variant 
	: std::bool_constant<is_base_of_template<T, augs::trivial_variant>::value>
{
};

template <class T>
struct is_bitset : std::false_type {

};

template <class T>
struct is_bitset<augs::enum_bitset<T>> : std::true_type {

};

template <size_t I>
struct is_bitset<std::bitset<I>> : std::true_type {

};

template <class T>
constexpr bool is_bitset_v = is_bitset<T>::value;

template <class T, class = void>
struct is_introspective_leaf : std::false_type {
};

template <class T>
struct is_introspective_leaf<T, 
	std::enable_if_t<
		std::is_enum_v<T> 
		|| std::is_arithmetic_v<T> 
		|| is_bitset_v<T>
	>
> : std::true_type {

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

template <class StreamType>
struct can_stream_left_predicate {
	template <class T>
	static constexpr bool value = can_stream_left_v<StreamType, T>;
};

template <class StreamType>
struct can_stream_right_predicate {
	template <class T>
	static constexpr bool value = can_stream_right_v<StreamType, T>;
};

template <class T>
struct exclude_no_type : std::false_type {

};