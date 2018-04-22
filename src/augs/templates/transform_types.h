#pragma once
#include <type_traits>
#include "augs/templates/type_list.h"

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