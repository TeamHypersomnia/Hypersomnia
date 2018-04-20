#pragma once
#include <type_traits>
#include <utility>

#include "augs/templates/predicate_templates.h"
#include "augs/templates/nth_type_in.h"
#include "augs/templates/list_utils.h"
#include "augs/templates/sequence_utils.h"
#include "augs/templates/type_list.h"

template <
	size_t Index,
	template <class...> class Criterion,
	class List
>
struct filter_types_detail;

template <
	size_t Index,
	template <class...> class Criterion,
	template <class...> class List
>
struct filter_types_detail<
	Index,
	Criterion,
	List<>
> { 
	using types = List<>; 
	using indices = std::index_sequence<>;
};

template <
	size_t Index,
	template <class...> class Criterion,
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

	using indices = std::conditional_t<
		Criterion<Head>::value,
		prepend_to_sequence_t<
			Index,
			typename filter_types_detail<Index + 1, Criterion, List<Tail...>>::indices
		>,
		typename filter_types_detail<Index + 1, Criterion, List<Tail...>>::indices
	>;

	static constexpr bool found = indices().size() > 0;

	template <size_t I>
	using get_type = nth_type_in_list_t<I, types>;
};

template <
	template <class...> class Criterion,
	class List
>
using filter_types_in_list = filter_types_detail<0, Criterion, List>;

template <
	template <class...> class Criterion,
	class List
>
using filter_types_in_list_t = typename filter_types_detail<0, Criterion, List>::types;

template <
	template <class...> class Criterion,
	class... Args
>
using filter_types = filter_types_in_list_t<Criterion, type_list<Args...>>;

template <
	template <class...> class Criterion,
	class List
>
using find_matching_type_in_list = typename filter_types_in_list<Criterion, List>::template get_type<0>;

template <class S, class List>
constexpr bool is_one_of_list_v = filter_types_in_list<bind_types<std::is_same, S>::template type, List>::found;

template <class S, class... Types>
constexpr bool is_one_of_v = is_one_of_list_v<S, type_list<Types...>>;

template <class A, class B>
using detail_derived_from = std::is_base_of<B, A>;

template <class S, class List>
constexpr bool is_derived_from_any_of_list_v = filter_types_in_list<bind_types<detail_derived_from, S>::template type, List>::found;

template <class S, class... Types>
constexpr bool is_derived_from_any_of_v = is_derived_from_any_of_list_v<S, type_list<Types...>>;

template <class S, class List>
constexpr size_t index_in_list_v = sequence_element_v<0, typename filter_types_in_list<bind_types<std::is_same, S>::template type, List>::indices>;

template <class S, class... Types>
constexpr size_t index_in_v = index_in_list_v<S, type_list<Types...>>;

template <template <class...> class Criterion, class List>
constexpr size_t count_matches_in_list_v = filter_types_in_list<Criterion, List>::indices::size();

template <class S, class List>
constexpr size_t count_occurences_in_list_v = count_matches_in_list_v<bind_types<std::is_same, S>::template type, List>;

template <class S, class... Types>
constexpr size_t count_occurences_in_v = count_occurences_in_list_v<S, type_list<Types...>>;

template <template <class...> class Criterion, class List>
constexpr size_t match_exists_in_list_v = count_matches_in_list_v<Criterion, List> > 0;

template <template <class...> class Criterion, class List>
constexpr size_t all_are_v = count_matches_in_list_v<Criterion, List> == num_types_in_list_v<List>;