#pragma once
#include <array>

template <class T, class = void>
struct has_key_type : std::false_type {};

template <class T>
struct has_key_type<T, decltype(typename T::key_type(), void())> : std::true_type {};


template <class T, class = void>
struct has_mapped_type : std::false_type {};

template <class T>
struct has_mapped_type<T, decltype(typename T::mapped_type(), void())> : std::true_type {};


template <class T, class = void>
struct has_value_type : std::false_type {};

template <class T>
struct has_value_type<T, decltype(typename T::value_type(), void())> : std::true_type {};


template <class T, class = void>
struct can_access_data : std::false_type {};

template <class T>
struct can_access_data<T, decltype(std::declval<T>().data(), void())> : std::true_type {};


template <class T, class = void>
struct can_reserve : std::false_type {};

template <class T>
struct can_reserve<T, decltype(std::declval<T>().reserve(0u), void())> : std::true_type {};


template <class T, class = void>
struct can_clear : std::false_type {};

template <class T>
struct can_clear<T, decltype(std::declval<T>().clear(), void())> : std::true_type {};


template<typename Trait>
struct size_test_detail
{
	static Trait ttt;

	template<int Value = ttt.size()>
	static std::true_type do_call(int){ return std::true_type(); }

	static std::false_type do_call(...){ return std::false_type(); }

	static auto call(){ return do_call(0); }
};

template<typename Trait>
struct size_test : decltype(size_test_detail<Trait>::call()) {

};

template <class T>
struct is_std_array : std::false_type {};

template <class T, std::size_t I>
struct is_std_array<std::array<T, I>> : std::true_type {};

template <class T>
constexpr bool is_std_array_v = is_std_array<T>::value;

template <class T>
constexpr bool has_key_type_v = has_key_type<T>::value;

template <class T>
constexpr bool has_mapped_type_v = has_mapped_type<T>::value;

template <class T>
constexpr bool has_value_type_v = has_value_type<T>::value;

template <class T>
constexpr bool can_access_data_v = can_access_data<T>::value;

template <class T>
constexpr bool can_reserve_v = can_reserve<T>::value;

template <class T>
constexpr bool can_clear_v = can_clear<T>::value;

template <class T>
constexpr bool is_associative_container_v = has_key_type_v<T> && has_mapped_type_v<T>;

template <class T>
constexpr bool is_unary_container_v = !is_std_array_v<T> && has_value_type_v<T> && !is_associative_container_v<T>;

template <class T>
constexpr bool is_constexpr_size_container_v = size_test<T>::value;

template <class T>
constexpr bool is_container_v = is_unary_container_v<T> || is_associative_container_v<T>;

template <class T>
constexpr bool is_variable_size_container_v = is_container_v<T> && !is_constexpr_size_container_v<T>;
