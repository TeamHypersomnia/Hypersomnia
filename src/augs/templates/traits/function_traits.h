#pragma once
#include <cstddef>
#include "augs/templates/nth_type_in.h"

template <class T, class R, class... Args>
struct make_function_traits {
	template <std::size_t I>
	struct arg {
		using type = nth_type_in_t<I, Args...>;
	};

	using last_arg_t = nth_type_in_t<sizeof...(Args) - 1, Args...>;
};

template <class T>
struct function_traits : function_traits<decltype(&T::operator())> {};

template <class R, class... Args>
struct function_traits<R(Args...)> : make_function_traits<void, R, Args...> {};

template <class T, class R, class... Args>
struct function_traits<R(T::*)(Args...) const> : make_function_traits<const T, R, Args...> {};

template <class T, class R, class... Args>
struct function_traits<R(T::*)(Args...)> : make_function_traits<T, R, Args...> {};

template <class T, std::size_t I>
using argument_t = typename function_traits<T>::template arg<I>::type;

template <class T>
using last_argument_t = typename function_traits<T>::last_arg_t;