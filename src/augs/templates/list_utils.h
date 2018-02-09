#pragma once
#include <utility>

template <class, class>
struct prepend_to_list;

template <class T, template <class...> class List, class... Args>
struct prepend_to_list<T, List<Args...>> {
	using type = List<T, Args...>;
};

template <class Element, class List>
using prepend_to_list_t = typename prepend_to_list<Element, List>::type;

template <class, class>
struct append_to_list;

template <class T, template <class...> class List, class... Args>
struct append_to_list<T, List<Args...>> {
	using type = List<Args..., T>;
};

template <class Element, class List>
using append_to_list_t = typename append_to_list<Element, List>::type;


template <size_t, class>
struct add_to_sequence;

template <size_t I, size_t... Is>
struct add_to_sequence<I, std::index_sequence<Is...>> {
	using type = std::index_sequence<I, Is...>;
};

template <size_t I, class T>
using add_to_sequence_t = typename add_to_sequence<I, T>::type;

template <size_t I, class T>
struct sequence_element;

template <size_t I, class T, T Head, T... Tail>
struct sequence_element<I, std::integer_sequence<T, Head, Tail...>>
	: sequence_element<I - 1, std::integer_sequence<T, Tail...>> { };

template <class T, T Head, T... Tail>
struct sequence_element<0, std::integer_sequence<T, Head, Tail...>> {
	static constexpr T value = Head;
};

template <size_t I, class T>
static constexpr auto sequence_element_v = sequence_element<I, T>::value;

template <class>
struct concatenate_lists;

template <template <class...> class A, class... Args>
struct concatenate_lists<A<Args...>> {
	template <class>
	struct result;
	
	template <template <class...> class B, class... Brgs>
	struct result<B<Brgs...>> {
		using type = A<Args..., Brgs...>;
	};
};

template <class A, class B>
using concatenate_lists_t = typename concatenate_lists<A>::template result<B>::type;
