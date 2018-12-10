#pragma once
#include <utility>

template <class T>
struct type_of_in_place_type;

template <class T>
struct type_of_in_place_type<std::in_place_type_t<T>> {
	using type = T;
};

template <class T>
using type_of_in_place_type_t = typename type_of_in_place_type<T>::type;
