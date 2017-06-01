#pragma once
#include "augs/templates/constexpr_if.h"

template <class T, T... x>
struct _vals;

template <class... T>
struct _types;

template <bool val, class Types, class... Vals>
struct static_assert_print {
	Types d;
};

template <class Types, class... Vals>
struct static_assert_print<true, Types, Vals...> {};
