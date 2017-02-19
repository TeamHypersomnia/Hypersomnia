#pragma once
namespace std {
	template <class...>
	class tuple;
}

namespace templates_detail
{
	template<int... Is>
	struct seq { };

	template<int N, int... Is>
	struct gen_seq : gen_seq<N - 1, N - 1, Is...> { };

	template<int... Is>
	struct gen_seq<0, Is...> : seq<Is...> { };

	template<typename T, typename F, int... Is>
	void for_each(T&& t, F f, seq<Is...>)
	{
		auto l = { (f(std::get<Is>(t)), 0)... };
	}

	template<typename T, typename F, int... Is>
	void for_eaches(T&& t, T&& b, F f, seq<Is...>)
	{
		auto l = { (f(std::get<Is>(t), std::get<Is>(b)), 0)... };
	}
}

template<
	template<typename...> class List,
	typename... Ts, 
	typename F
>
void for_each_in_tuple(const List<Ts...>& t, F f)
{
	templates_detail::for_each(t, f, templates_detail::gen_seq<sizeof...(Ts)>());
}

template<
	template<typename...> class List,
	typename... Ts,
	typename F
>
void for_each_in_tuple(List<Ts...>& t, F f)
{
	templates_detail::for_each(t, f, templates_detail::gen_seq<sizeof...(Ts)>());
}

template<
	template<typename...> class List,
	typename... Ts,
	typename F
>
void for_each_in_tuples(const List<Ts...>& t, const List<Ts...>& b, F f)
{
	templates_detail::for_eaches(t, b, f, templates_detail::gen_seq<sizeof...(Ts)>());
}

template<
	template<typename...> class List,
	typename... Ts,
	typename F
>
void for_each_in_tuples(List<Ts...>& t, List<Ts...>& b, F f)
{
	templates_detail::for_eaches(t, b, f, templates_detail::gen_seq<sizeof...(Ts)>());
}

template<typename... Ts, typename F>
void for_each_type(F f)
{
	templates_detail::for_each(std::tuple<Ts...>(), f, templates_detail::gen_seq<sizeof...(Ts)>());
}
