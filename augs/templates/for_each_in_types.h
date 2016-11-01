#pragma once
#include <tuple>

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

template<typename... Ts, typename F>
void for_each_in_tuple(const std::tuple<Ts...>& t, F f)
{
	templates_detail::for_each(t, f, templates_detail::gen_seq<sizeof...(Ts)>());
}

template<typename... Ts, typename F>
void for_each_in_tuple(std::tuple<Ts...>& t, F f)
{
	templates_detail::for_each(t, f, templates_detail::gen_seq<sizeof...(Ts)>());
}

template<typename... Ts, typename F>
void for_each_in_tuples(const std::tuple<Ts...>& t, const std::tuple<Ts...>& b, F f)
{
	templates_detail::for_eaches(t, b, f, templates_detail::gen_seq<sizeof...(Ts)>());
}

template<typename... Ts, typename F>
void for_each_in_tuples(std::tuple<Ts...>& t, std::tuple<Ts...>& b, F f)
{
	templates_detail::for_eaches(t, b, f, templates_detail::gen_seq<sizeof...(Ts)>());
}

template<typename... Ts, typename F>
void for_each_type(F f)
{
	templates_detail::for_each(std::tuple<Ts...>(), f, templates_detail::gen_seq<sizeof...(Ts)>());
}
