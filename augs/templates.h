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

template<class T, class L>
void erase_remove(std::vector<T>& v, const L& l) {
	v.erase(std::remove_if(v.begin(), v.end(), l), v.end());
}

template <typename T>
void serialize(std::ofstream& f, const T& t) {
	f.write((const char*)&t, sizeof(T));
}

template <typename T>
void deserialize(std::ifstream& f, T& t) {
	f.read((char*)&t, sizeof(T));
}

template <typename T>
void serialize_vector(std::ofstream& f, const std::vector<T>& t) {
	size_t size = t.size();
	f.write((const char*)&size, sizeof(size));

	for (size_t i = 0; i < size; ++i)
		f.write((const char*)&t[i], sizeof(T));
}

template <typename T>
void deserialize_vector(std::ifstream& f, std::vector<T>& t) {
	size_t size = 0;
	f.read((char*)&size, sizeof(size));

	for (size_t i = 0; i < size; ++i) {
		T obj;
		f.read((char*)&obj, sizeof(T));
		t.emplace_back(obj);
	}
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

namespace detail
{
	template<int... Is>
	struct seq { };

	template<int N, int... Is>
	struct gen_seq : gen_seq<N - 1, N - 1, Is...> { };

	template<int... Is>
	struct gen_seq<0, Is...> : seq<Is...> { };
}

namespace detail
{
	template<typename T, typename F, int... Is>
	void for_each(T&& t, F f, seq<Is...>)
	{
		auto l = { (f(std::get<Is>(t)), 0)... };
	}
}

template<typename... Ts, typename F>
void for_each_in_tuple(std::tuple<Ts...> const& t, F f)
{
	detail::for_each(t, f, detail::gen_seq<sizeof...(Ts)>());
}

template<template<typename...> class List,
	template<typename> class Mod,
	typename ...Args>
	struct transform_types {
	typedef List<typename Mod<Args>::type...> type;
};