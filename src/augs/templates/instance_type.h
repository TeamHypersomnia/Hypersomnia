#pragma once
#include <type_traits>
#include "augs/templates/type_matching_and_indexing.h"

template <class T>
using instance_of = typename T::instance;

template <class SearchedInstance, class MetaCandidate, class = void>
struct instance_type_matches {

};

template <class SearchedInstance, class MetaCandidate>
struct instance_type_matches<
	SearchedInstance,
	MetaCandidate,
	decltype(typename MetaCandidate::instance(), void())
> : std::bool_constant<std::is_same_v<typename MetaCandidate::instance, SearchedInstance>> {
};

template <class T, class MetaTuple>
decltype(auto) get_meta_of(T&&, MetaTuple&& t) {
	return std::get<
		find_matching_type_in_list<
			bind_types_t<instance_type_matches, std::decay_t<T>>, std::decay_t<MetaTuple>
		>
	>(std::forward<MetaTuple>(t));
}