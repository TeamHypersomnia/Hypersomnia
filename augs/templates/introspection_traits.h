#pragma once

struct true_returner {
	template <class... Types>
	bool operator()(Types...) const {
		return true;
	}
};

template <class T, class = void>
struct has_introspect {
	static constexpr bool value = false;
};

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
> {
	static constexpr bool value = true;
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

//template <class StreamType>
//constexpr bool can_stream_left_predicate_v = can_stream_left_predicate<StreamType>;

template <class StreamType>
struct can_stream_right_predicate {
	template <class T>
	static constexpr bool value = can_stream_right_v<StreamType, T>;
};

//template <class StreamType>
//constexpr bool can_stream_right_predicate_v = can_stream_right_predicate<StreamType>;


template <class T>
struct exclude_no_type {
	static constexpr bool value = false;
};