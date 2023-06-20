#pragma once
#include "augs/templates/identity_templates.h"

template <class T, class = void>
struct has_enum_to_string : std::false_type {};

template <class T>
struct has_enum_to_string<T, decltype(augs::enum_to_string(T()), void())> 
	: std::true_type 
{};

template <class T>
constexpr bool has_enum_to_string_v = has_enum_to_string<T>::value;
