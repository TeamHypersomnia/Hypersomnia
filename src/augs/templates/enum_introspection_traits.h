#pragma once
#include "augs/templates/always_false.h"

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
