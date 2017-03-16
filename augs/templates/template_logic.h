#pragma once
#include <type_traits>

template <
	template <class...> class operation,
	template <class...> class... Types
>
struct of_templates {
	template <class T>
	struct predicate 
		: std::bool_constant<operation<Types<T>...>::value>
	{
	};
};

template <template <class...> class... Types>
using template_disjunction_t = typename of_templates<std::disjunction, Types...>::predicate;