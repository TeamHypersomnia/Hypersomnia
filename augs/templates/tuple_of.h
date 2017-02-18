#pragma once
#include <tuple>

template<class T>
struct empty_mod {
	typedef T type;
};

template<template<typename> class Mod,
	typename ...Args>
	struct tuple_of {
	typedef std::tuple<typename Mod<Args>::type...> type;
};

template<template<typename> class Mod,
	typename ...Args>
	using tuple_of_t = typename tuple_of<Mod, Args...>::type;