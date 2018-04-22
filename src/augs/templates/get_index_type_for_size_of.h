#pragma once
#include <limits>

#include "augs/templates/filter_types.h"
#include "augs/templates/type_list.h"

template <class SizedType>
struct is_size_sufficient {
	template <class IndexType>
	struct type : std::bool_constant<std::numeric_limits<IndexType>::max() >= sizeof(SizedType) - 1> {};
};

template <class T>
using get_index_type_for_size_of_t = find_matching_type_in_list<
	is_size_sufficient<T>::template type,
	type_list<unsigned char, unsigned short, unsigned int>
>;
