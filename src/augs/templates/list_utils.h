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
