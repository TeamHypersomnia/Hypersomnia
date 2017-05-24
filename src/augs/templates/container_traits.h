#pragma once

template <class T, class = void>
struct has_key_type : std::bool_constant<false> {};

template <class T>
struct has_key_type<T, decltype(typename T::key_type(), void())> : std::bool_constant<true> {};

template <class T>
constexpr bool has_key_type_v = has_key_type<T>::value;


template <class T, class = void>
struct has_mapped_type : std::bool_constant<false> {};

template <class T>
struct has_mapped_type<T, decltype(typename T::mapped_type(), void())> : std::bool_constant<true> {};

template <class T>
constexpr bool has_mapped_type_v = has_mapped_type<T>::value;


template <class T, class = void>
struct has_value_type : std::bool_constant<false> {};

template <class T>
struct has_value_type<T, decltype(typename T::value_type(), void())> : std::bool_constant<true> {};

template <class T>
constexpr bool has_value_type_v = has_value_type<T>::value;


template <class T, class = void>
struct can_access_data : std::bool_constant<false> {};

template <class T>
struct can_access_data<T, decltype(std::declval<T>().data(), void())> : std::bool_constant<true> {};

template <class T>
constexpr bool can_access_data_v = can_access_data<T>::value;


template <class T>
constexpr bool is_associative_container_v = has_key_type_v<T> && has_value_type_v<T>;

template <class T>
constexpr bool is_unary_container = has_value_type_v<T> && !is_associative_container_v<T>;
