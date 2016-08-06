#pragma once
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <functional>
#include <bitset>
#include <tuple>

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

template<class Container, class T>
void erase_remove(Container& v, const T& l) {
	v.erase(std::remove_if(v.begin(), v.end(), l), v.end());
}

template<class Container, class T>
void remove_element(Container& v, const T& l) {
	v.erase(std::remove(v.begin(), v.end(), l), v.end());
}

template<class Container, class T>
bool found_in(Container& v, const T& l) {
	return std::find(v.begin(), v.end(), l) != v.end();
}

template<class Container, class T>
auto find_in(Container& v, const T& l) {
	return std::find(v.begin(), v.end(), l);
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
struct maybe_const_ref { typedef std::conditional_t<is_const, const T&, T&> type; };

template<bool is_const, class T>
using maybe_const_ref_t = typename maybe_const_ref<is_const, T>::type;

template<bool is_const, class T>
struct maybe_const_ptr { typedef std::conditional_t<is_const, const T*, T*> type; };

template<bool is_const, class T>
using maybe_const_ptr_t = typename maybe_const_ptr<is_const, T>::type;

template<typename T, typename = void>
struct is_component_synchronized : std::false_type { };

template<typename T>
struct is_component_synchronized<T, decltype(std::declval<T>().activated, void())> : std::true_type { };

//template <typename Base, typename Tuple, std::size_t I = 0>
//struct tuple_ref_index;
//
//template <typename Base, typename Head, typename... Tail, std::size_t I>
//struct tuple_ref_index<Base, std::tuple<Head, Tail...>, I>
//	: std::conditional<std::is_base_of<Base, Head>::value
//	, std::integral_constant<std::size_t, I>
//	, tuple_ref_index<Base, std::tuple<Tail...>, I + 1>
//	>::type
//{
//};
//
//template <typename Base, typename Tuple>
//auto tuple_ref_by_inheritance(Tuple&& tuple)
//-> decltype(std::get<tuple_ref_index<Base, typename std::decay<Tuple>::type>::value>(std::forward<Tuple>(tuple)))
//{
//	return std::get<tuple_ref_index<Base, typename std::decay<Tuple>::type>::value>(std::forward<Tuple>(tuple));
//}

template <class T>
struct saved_args_base {

};

template <class T, class... Args>
struct saved_args : saved_args_base<T> {
	std::tuple<Args...> args;
};

template <class T>
struct save_args {
	template<class... Args>
	auto operator()(Args... args) {
		return saved_args<T, Args...>(std::make_tuple(args...));
	}
};

namespace detail {
	template <class F, class Tuple, std::size_t... I>
	constexpr decltype(auto) apply_impl(F&& f, Tuple&& t, std::index_sequence<I...>)
	{
		return std::invoke(std::forward<F>(f), std::get<I>(std::forward<Tuple>(t))...);
		// Note: std::invoke is a C++17 feature
	}
} // namespace detail

namespace std {
	template <class F, class Tuple>
	constexpr decltype(auto) apply(F&& f, Tuple&& t)
	{
		return detail::apply_impl(std::forward<F>(f), std::forward<Tuple>(t),
			std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>::value>{});
	}
}

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