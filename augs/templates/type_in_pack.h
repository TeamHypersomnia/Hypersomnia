#pragma once
#include "minimal_templates.h"

namespace std {
	template <class...>
	class tuple;
}

template <typename T, typename List>
struct has_type;

template <typename T, template <class...> class List>
struct has_type<T, List<>> : std::false_type {};

template <typename T, typename U, template <class...> class List, typename... Ts>
struct has_type<T, List<U, Ts...>> : has_type<T, List<Ts...>> {};

template <typename T, template <class...> class List, typename... Ts>
struct has_type<T, List<T, Ts...>> : std::true_type {};

template <typename T, typename List>
using list_contains_type = typename has_type<T, List>::type;

template <class T, class List>
struct index_in_list;

template <class T, template <class...> class List, class... Types>
struct index_in_list<T, List<T, Types...>> {
	static const std::size_t value = 0;
};

template <class T, class U, template <class...> class List, class... Types>
struct index_in_list<T, List<U, Types...>> {
	static_assert(list_contains_type<T, List<U, Types...>>::value, "No such type in the tuple or parameter pack!");

	static const std::size_t value = 1 + index_in_list<T, List<Types...>>::value;
};

template <typename T, typename... Types>
using pack_contains_type = typename has_type<T, std::tuple<Types...>>::type;

template <typename T, typename List>
struct nth_type_in;

template<unsigned idx, class... Types>
struct nth_type_in_pack {
	static_assert(idx < sizeof...(Types), "Type index out of bounds!");
	typedef std::decay_t<decltype(std::get<idx>(std::tuple<Types...>()))> type;
};

template<unsigned idx, class... Types>
using nth_type_in_pack_t = typename nth_type_in_pack<idx, Types...>::type;

template<class T, class... Types>
struct index_in_pack {
	static const size_t value = index_in_list<T, std::tuple<Types...>>::value;
};
