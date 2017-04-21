#pragma once

template <class, class>
struct concat_lists;

template <
	template <class...> class ListA,
	template <class...> class ListB,
	class... ArgsA,
	class... ArgsB
>
struct concat_lists<
	ListA<ArgsA...>,
	ListB<ArgsB...>
> {
	using type = ListA<ArgsA..., ArgsB...>;
};

template <class A, class B>
using concat_lists_t = typename concat_lists<A, B>::type;