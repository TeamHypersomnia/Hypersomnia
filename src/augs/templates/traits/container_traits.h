#pragma once
#include <type_traits>
#include <cstddef>
#include <utility>

#include "augs/templates/traits/is_std_array.h"
#include "augs/templates/has_begin_and_end.h"

template <int t>
struct constexpr_tester {};

template <class T, class = void>
struct has_constexpr_capacity : std::false_type {};

template <class T>
struct has_constexpr_capacity<T, decltype(constexpr_tester<T::capacity()>(), void())> : std::true_type {};

template <class T>
constexpr bool has_constexpr_capacity_v = has_constexpr_capacity<T>::value;


template <class T, class = void>
struct is_associative : std::false_type {};

template <class T>
struct is_associative<T, decltype(typename T::key_type(), typename T::mapped_type(), void())> : std::true_type {};


template <class T, class K, class = void>
struct has_member_find : std::false_type {};

template <class T, class K>
struct has_member_find<T, K, decltype(std::declval<T&>().find(std::declval<const K&>()), void())> : std::true_type {};


template <class T, class = void>
struct can_access_data : std::false_type {};

template <class T>
struct can_access_data<T, decltype(std::declval<T&>().data(), void())> : std::true_type {};


template <class T, class = void>
struct can_reserve : std::false_type {};

template <class T>
struct can_reserve<T, decltype(std::declval<T&>().reserve(0u), void())> : std::true_type {};


template <class T, class = void>
struct can_resize : std::false_type {};

template <class T>
struct can_resize<T, decltype(std::declval<T&>().resize(0u), void())> : std::true_type {};


template <class T, class = void>
struct can_clear : std::false_type {};

template <class T>
struct can_clear<T, decltype(std::declval<T&>().clear(), void())> : std::true_type {};


template <class T, class = void>
struct can_emplace_back : std::false_type {};

template <class T>
struct can_emplace_back<T, decltype(std::declval<T>().emplace_back(), void())> : std::true_type {};


template <class A, class B, class = void>
struct has_suitable_member_assign : std::false_type {};

template <class A, class B>
struct has_suitable_member_assign<A, B, decltype(std::declval<A&>().assign(std::declval<B&>().begin(), std::declval<B&>().end()), void())> : std::true_type {};


template <class T>
constexpr bool can_access_data_v = can_access_data<T>::value;

template <class T, class K>
constexpr bool has_member_find_v = has_member_find<T, K>::value;

template <class T, class K>
constexpr bool member_find_returns_ptr_v = std::is_pointer_v<decltype(std::declval<T&>().find(std::declval<const K&>()))>;

template <class T>
constexpr bool can_reserve_v = can_reserve<T>::value;

template <class T>
constexpr bool can_resize_v = can_resize<T>::value;

template <class T>
constexpr bool can_clear_v = can_clear<T>::value;

template <class T>
constexpr bool can_emplace_back_v = can_emplace_back<T>::value;

template <class T>
constexpr bool is_associative_v = is_associative<T>::value;

template <class T>
constexpr bool is_container_v = has_begin_and_end_v<T> && !is_std_array_v<T>;

template <class A, class B>
constexpr bool has_suitable_member_assign_v = has_suitable_member_assign<A, B>::value;