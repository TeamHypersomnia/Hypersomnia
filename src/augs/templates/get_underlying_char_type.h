#pragma once
#include <string>

template <class T>
struct get_underlying_char_type {
	typedef T type;
};

template <class T>
struct get_underlying_char_type<std::basic_string<T>> {
	typedef T type;
};

template <class T>
struct get_underlying_char_type<const T*> {
	typedef T type;
};

template <class T>
using get_underlying_char_type_t = typename get_underlying_char_type<T>::type;

template <class T>
using get_string_for_t = typename std::basic_string<get_underlying_char_type_t<T>>;