#pragma once
#include <cstddef>
#include <type_traits>
#include "augs/templates/nth_type_in.h"
#include "augs/templates/list_utils.h"

template <
	size_t Index,
	template <class> class Criterion,
	class List
>
struct filter_types_detail;

template <
	size_t Index,
	template <class> class Criterion,
	template <class...> class List
>
struct filter_types_detail<
	Index,
	Criterion,
	List<>
> { 
	using types = List<>; 
};

template <
	size_t Index,
	template <class> class Criterion,
	template <class...> class List,
	class Head,
	class... Tail
>
struct filter_types_detail<
	Index,
	Criterion,
	List<Head, Tail...>
> {
	using types = std::conditional_t<
		Criterion<Head>::value,
		prepend_to_list_t<
			Head,
			typename filter_types_detail<Index + 1, Criterion, List<Tail...>>::types
		>,
		typename filter_types_detail<Index + 1, Criterion, List<Tail...>>::types
	>;

	template <size_t I>
	using get_type = nth_type_in_list_t<I, types>;
};

template <
	template <class> class Criterion,
	class List
>
using filter_types_in_list = filter_types_detail<0, Criterion, List>;

template <
	template <class> class Criterion,
	class List
>
using filter_types_in_list_t = typename filter_types_detail<0, Criterion, List>::types;

template <
	template <class> class Criterion,
	class List
>
using find_matching_type_in_list = typename filter_types_in_list<Criterion, List>::template get_type<0>;
