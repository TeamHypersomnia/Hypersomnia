#pragma once
#include <type_traits>

namespace templates_detail {
	template<typename T>
	struct typed {
		typedef T type;
	};
}

template <template<class, class> class Criterion, typename T, typename... >
struct find_matching_type;

template <template<class, class> class Criterion, typename T>
struct find_matching_type<Criterion, T> {};

template <template<class, class> class Criterion, typename T, typename U, typename... Ts >
struct find_matching_type<Criterion, T, U, Ts...> :
	std::conditional_t<Criterion<T, U>::value, ::templates_detail::typed<U>, find_matching_type<Criterion, T, Ts...>> {};

template<class S, class... T>
using find_convertible_type = typename find_matching_type<std::is_convertible, S, T...>::type;