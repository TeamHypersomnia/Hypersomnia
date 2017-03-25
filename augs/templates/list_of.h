#pragma once
#include "augs/misc/trivially_copyable_tuple.h"

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
using tuple_of_t = typename list_of_t<std::tuple, Mod, Args...>;