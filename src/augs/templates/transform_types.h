#pragma once
#include <type_traits>
#include "augs/templates/type_list.h"
#include "augs/templates/type_matching_and_indexing.h"

template <class List>
struct reverse_types_in_list;

template <
	template <class...> class List,
	class T,
	class... Args
>
struct reverse_types_in_list<List<T, Args...>> {
	using type = append_to_list_t<T, typename reverse_types_in_list<List<Args...>>::type>;
};

template <
	template <class...> class List
>
struct reverse_types_in_list<List<>> {
	using type = List<>;
};

template <class List>
using reverse_types_in_list_t = typename reverse_types_in_list<List>::type;

template <
	class List,
	template <class> class Mod
>
struct transform_types_in_list;

template <
	template <class...> class List,
	template <class> class Mod,
	class... Args
>
struct transform_types_in_list<List<Args...>, Mod> {
	using type = List<Mod<Args>...>;
};

template <
	class List, 
	template <class...> class To
>
struct replace_list_type;

template <
	template <class...> class List,
	template <class...> class To,
	class... Args 
>
struct replace_list_type<List<Args...>, To> {
	using type = To<Args...>;
};

template <
	class List,
	template <class...> class To
>
using replace_list_type_t = typename replace_list_type<List, To>::type;

template <
	class List,
	template <class> class Mod
>
using transform_types_in_list_t = typename transform_types_in_list<List, Mod>::type;