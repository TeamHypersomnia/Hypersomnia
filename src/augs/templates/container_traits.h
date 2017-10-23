#pragma once
#include <array>

template <class T, class = void>
struct is_associative : std::false_type {};

template <class T>
struct is_associative<T, decltype(typename T::key_type(), typename T::mapped_type(), void())> : std::true_type {};


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


template <class T, class = void>
struct can_emplace_back : std::false_type {};

template <class T>
struct can_emplace_back<T, decltype(std::declval<T>().emplace_back(), void())> : std::true_type {};


template <class T, class = void>
struct has_begin_and_end : std::false_type {};

template <class T>
struct has_begin_and_end<T, decltype(std::declval<T>().begin(), std::declval<T>().end(), void())> : std::true_type {};


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
constexpr bool can_access_data_v = can_access_data<T>::value;

template <class T>
constexpr bool can_reserve_v = can_reserve<T>::value;

template <class T>
constexpr bool can_clear_v = can_clear<T>::value;

template <class T>
constexpr bool can_emplace_back_v = can_emplace_back<T>::value;

template <class T>
constexpr bool is_associative_v = is_associative<T>::value;

template <class T>
constexpr bool is_constexpr_size_v = size_test<T>::value;

template <class T>
constexpr bool is_container_v = has_begin_and_end<T>::value && !is_std_array_v<T>;

template <class T>
constexpr bool is_variable_size_container_v = is_container_v<T> && !is_constexpr_size_v<T>;
