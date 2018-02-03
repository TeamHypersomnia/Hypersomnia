#pragma once
#include "augs/templates/type_list.h"
#include "augs/templates/type_matching_and_indexing.h"

template <class A, class B>
struct type_pair {
	using First = A;
	using Second = B;
};

template <class L>
struct type_map_impl {
	template <class T>
	struct at_impl {
		template <class Candidate>
		struct match_first : std::bool_constant<std::is_same_v<typename Candidate::First, T>> {};

		using type = typename find_matching_type_in_list<L, match_first>::Second;
	};

	template <class S>
	using at = typename at_impl<S>::type;
};

template <class... Pairs>
using type_map = type_map_impl<type_list<Pairs...>>;

template <class Map, class T>
using type_at = typename Map::template at<T>;
