#pragma once

template <class T>
struct type_argument;

template <template <class> class A, class T>
struct type_argument<A<T>> {
	using type = T;
};

template <class T>
using type_argument_t = typename type_argument<T>::type;
