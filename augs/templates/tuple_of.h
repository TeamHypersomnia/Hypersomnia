#pragma once
#include "augs/misc/trivial_tuple.h"

template<class T>
struct empty_mod {
	typedef T type;
};

template <
	template<typename...> class List,
	template<typename> class Mod,
	typename ...Args
>
struct list_of {
	typedef List<typename Mod<Args>::type...> type;
};

template <
	template<typename...> class List,
	template<typename> class Mod,
	typename ...Args
>
using list_of_t = typename list_of<List, Mod, Args...>::type;

template<
	template<typename> class Mod,
	typename ...Args
>
using tuple_of_t = typename list_of<std::tuple, Mod, Args...>::type;

template<
	template<typename> class Mod,
	typename ...Args
>
using trivial_tuple_of_t = typename list_of<augs::trivial_tuple, Mod, Args...>::type;