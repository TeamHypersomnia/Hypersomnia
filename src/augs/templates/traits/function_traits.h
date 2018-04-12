#pragma once
#include "augs/templates/type_matching_and_indexing.h"

template <class T>
struct function_traits : function_traits<decltype(&T::operator())>
{};

template <class T, class R, class... Args>
struct function_traits<R(T::*)(Args...) const> {
	template <std::size_t I>
	struct arg {
		using type = nth_type_in_t<I, Args...>;
	};
};

template <class T, std::size_t I>
using argument_t = typename function_traits<T>::template arg<I>::type;