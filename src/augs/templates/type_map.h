#pragma once
#include "augs/templates/type_list.h"
#include "augs/templates/type_pair.h"
#include "augs/templates/filter_types.h"

template <class L>
struct type_map_impl {
	template <class T>
	struct at_impl {
		template <class Candidate>
		struct match_first : std::bool_constant<std::is_same_v<typename Candidate::First, T>> {};

		using type = typename find_matching_type_in_list<match_first, L>::Second;
	};

	template <class S>
	using at = typename at_impl<S>::type;
};

template <class... Pairs>
using type_map = type_map_impl<type_list<Pairs...>>;

template <class Map, class T>
using type_at = typename Map::template at<T>;
