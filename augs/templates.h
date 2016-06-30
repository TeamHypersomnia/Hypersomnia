#pragma once
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <iomanip>

template <class T, class Tuple>
struct index_in_tuple;

template <class T, class... Types>
struct index_in_tuple<T, std::tuple<T, Types...>> {
	static const std::size_t value = 0;
};

template <class T, class U, class... Types>
struct index_in_tuple<T, std::tuple<U, Types...>> {
	static const std::size_t value = 1 + index_in_tuple<T, std::tuple<Types...>>::value;
};

template<class T, class... Types>
class index_in_pack {
	static const size_t value = index_in_tuple<T, std::tuple<Types...>>::value;
};

template<class T, class L>
void erase_remove(std::vector<T>& v, const L& l) {
	v.erase(std::remove_if(v.begin(), v.end(), l), v.end());
}

template<class T>
void remove_element(std::vector<T>& v, const T& l) {
	v.erase(std::remove(v.begin(), v.end(), l), v.end());
}

/* number to string conversion */

template <class T>
std::string to_string(T val) {
	std::ostringstream ss;
	ss << val;
	return ss.str();
}

template <>
std::string to_string(std::wstring val);

/* number to wide string conversion */

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

template <class T>
T to_value(std::wstring& s) {
	std::wistringstream ss(s);
	T v;
	ss >> v;
	return v;
}

template <class T>
T to_value(std::string& s) {
	std::istringstream ss(s);
	T v;
	ss >> v;
	return v;
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
}

template<typename... Ts, typename F>
void for_each_in_tuple(std::tuple<Ts...> const& t, F f)
{
	templates_detail::for_each(t, f, templates_detail::gen_seq<sizeof...(Ts)>());
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

template<bool _Test,
	template<typename> class _Ty1,
	template<typename> class _Ty2>
struct conditional_template
{	// type is _Ty2 for assumed !_Test
	template <typename T>
	using type = _Ty2<T>;
};

template<template<typename> class _Ty1,
	template<typename> class _Ty2>
struct conditional_template<true, _Ty1, _Ty2>
{	// type is _Ty1 for _Test
	template <typename T>
	using type = _Ty1<T>;
};

template<bool is_const, class T>
struct maybe_const_ref { typedef typename std::conditional<is_const, const T&, T&>::type type; };

template<bool is_const, class T>
struct maybe_const_ptr { typedef typename std::conditional<is_const, const T*, T*>::type type; };

template<typename T, typename = void>
struct is_component_synchronized : std::false_type { };

template<typename T>
struct is_component_synchronized<T, decltype(std::declval<T>().activated, void())> : std::true_type { };