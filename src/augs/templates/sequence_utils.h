#pragma once
#include "augs/templates/always_false.h"

template <size_t, class>
struct prepend_to_sequence;

template <size_t I, size_t... Is>
struct prepend_to_sequence<I, std::index_sequence<Is...>> {
	using type = std::index_sequence<I, Is...>;
};

template <size_t I, class T>
using prepend_to_sequence_t = typename prepend_to_sequence<I, T>::type;

template <size_t, class>
struct append_to_sequence;

template <size_t I, size_t... Is>
struct append_to_sequence<I, std::index_sequence<Is...>> {
	using type = std::index_sequence<Is..., I>;
};

template <size_t I, class T>
using append_to_sequence_t = typename append_to_sequence<I, T>::type;

template <size_t I, class T>
struct sequence_element;

template <size_t I, class T>
struct sequence_element<I, std::integer_sequence<T>> {
	static_assert(always_false_v<T>, "Error: an empty integer sequence was passed.");
};

template <size_t I, class T, T Head, T... Tail>
struct sequence_element<I, std::integer_sequence<T, Head, Tail...>>
	: sequence_element<I - 1, std::integer_sequence<T, Tail...>> { };

template <class T, T Head, T... Tail>
struct sequence_element<0, std::integer_sequence<T, Head, Tail...>> {
	static constexpr T value = Head;
};

template <size_t I, class T>
static constexpr auto sequence_element_v = sequence_element<I, T>::value;


template <class T>
struct reverse_sequence;

template <size_t A, size_t... elems>
struct reverse_sequence<std::index_sequence<A, elems...>> {
	using type = append_to_sequence_t<A, typename reverse_sequence<std::index_sequence<elems...>>::type>;
};

template <>
struct reverse_sequence<std::index_sequence<>> {
	using type = std::index_sequence<>;
};

template <class T>
using reverse_sequence_t = typename reverse_sequence<T>::type;

