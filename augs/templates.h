#pragma once
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <bitset>
#include <tuple>
#include <unordered_set>

template <typename T, typename Tuple>
struct has_type;

template <typename T>
struct has_type<T, std::tuple<>> : std::false_type {};

template <typename T, typename U, typename... Ts>
struct has_type<T, std::tuple<U, Ts...>> : has_type<T, std::tuple<Ts...>> {};

template <typename T, typename... Ts>
struct has_type<T, std::tuple<T, Ts...>> : std::true_type {};

template <typename T, typename Tuple>
using tuple_contains_type = typename has_type<T, Tuple>::type;

template <typename T, typename... Types>
using pack_contains_type = typename has_type<T, std::tuple<Types...>>::type;

template<unsigned idx, class... Types>
struct nth_type_in {
	static_assert(idx < sizeof...(Types), "Type index out of bounds!");
	typedef std::decay_t<decltype(std::get<idx>(std::tuple<Types...>()))> type;
};

template<unsigned idx, class... Types>
using nth_type_in_t = typename nth_type_in<idx, Types...>::type;

template <class T, class Tuple>
struct index_in_tuple;

template <class T, class... Types>
struct index_in_tuple<T, std::tuple<T, Types...>> {
	static const std::size_t value = 0;
};

template <class T, class U, class... Types>
struct index_in_tuple<T, std::tuple<U, Types...>> {
	static_assert(tuple_contains_type<T, std::tuple<U, Types...>>::value, "No such type in the tuple or parameter pack!");

	static const std::size_t value = 1 + index_in_tuple<T, std::tuple<Types...>>::value;
};

template<class T, class... Types>
struct index_in_pack {
	static const size_t value = index_in_tuple<T, std::tuple<Types...>>::value;
};

template<class Container, class T>
void erase_remove(Container& v, const T& l) {
	v.erase(std::remove_if(v.begin(), v.end(), l), v.end());
}

template<class Container, class T>
void remove_element(Container& v, const T& l) {
	v.erase(std::remove(v.begin(), v.end(), l), v.end());
}

template<class ContainerType, class T>
void remove_element(std::unordered_set<ContainerType>& v, const T& l) {
	v.erase(l);
}

template<class Container, class T>
bool found_in(Container& v, const T& l) {
	return std::find(v.begin(), v.end(), l) != v.end();
}

template<class Container, class T>
auto find_in(Container& v, const T& l) {
	return std::find(v.begin(), v.end(), l);
}

std::string to_string(std::wstring val);

template <class T>
std::wstring to_wstring(T val, int precision = -1, bool fixed = false) {
	std::wostringstream ss;

	if (precision > -1) {
		if (fixed) {
			ss << std::fixed;
		}

		ss << std::setprecision(precision);
	}

	ss << val;
	return ss.str();
}

std::wstring to_wstring(std::string val);

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

template<typename Container, typename T>
bool exists_in(const Container& c, T val) {
	return std::find(c.begin(), c.end(), val) != c.end();
}

template<template<typename...> class List,
	template<typename> class Mod,
	typename ...Args>
	struct transform_types {
	typedef List<typename Mod<Args>::type...> type;
};

template<template<typename> class Mod,
	typename ...Args>
	struct tuple_of {
	typedef std::tuple<typename Mod<Args>::type...> type;
};

template<template<typename> class Mod,
	typename ...Args>
	using tuple_of_t = typename tuple_of<Mod, Args...>::type;

template<bool is_const, class T>
struct maybe_const_ref { typedef std::conditional_t<is_const, const T&, T&> type; };

template<bool is_const, class T>
using maybe_const_ref_t = typename maybe_const_ref<is_const, T>::type;

template<bool is_const, class T>
struct maybe_const_ptr { typedef std::conditional_t<is_const, const T*, T*> type; };

template<bool is_const, class T>
using maybe_const_ptr_t = typename maybe_const_ptr<is_const, T>::type;

struct synchronizable_component;

template<typename T>
struct is_component_synchronized {
	static constexpr bool value = std::is_base_of<synchronizable_component, T>::value;
};

template <typename T>
struct has_held_ids_introspector
{
	struct dummy { /* something */ };

	template <typename C, typename P>
	static auto test(P * p) -> decltype(std::declval<C>().for_each_held_id(*p), std::true_type());

	template <typename, typename>
	static std::false_type test(...);

	typedef decltype(test<T, dummy>(nullptr)) type;
	static const bool value = std::is_same<std::true_type, decltype(test<T, dummy>(nullptr))>::value;
};

template <class T>
struct saved_args_base {

};

template <class T>
struct is_memcpy_safe {
	static const bool value = std::is_trivially_copyable<T>::value;
};

template <>
struct is_memcpy_safe<std::tuple<>> {
	static const bool value = true;
};

namespace augs {
	template <class... Args>
	class component_aggregate;
}

template <class... Args>
struct is_memcpy_safe<augs::component_aggregate<Args...>> {
	static const bool value = true;
};

template <class T, class... Args>
struct is_memcpy_safe<std::tuple<T, Args...>> {
	static const bool value = is_memcpy_safe<T>::value && is_memcpy_safe<std::tuple<Args...>>::value;
};

template <class A, class B>
struct is_memcpy_safe<std::pair<A, B>> {
	static const bool value = is_memcpy_safe<A>::value && is_memcpy_safe<B>::value;
};

template <class... Head>
struct are_types_memcpy_safe;

template <class Head>
struct are_types_memcpy_safe<Head> {
	static constexpr bool value = is_memcpy_safe<Head>::value;
};

template <class Head, class... Tail>
struct are_types_memcpy_safe<Head, Tail...> {
	static constexpr bool value =
		are_types_memcpy_safe<Head>::value
		&&
		are_types_memcpy_safe<Tail...>::value;
};

template<class Str, class Repl>
Str replace_all(Str str, Repl _from, Repl _to) {
	const Str& from(_from);
	const Str& to(_to);

	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != Str::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
}