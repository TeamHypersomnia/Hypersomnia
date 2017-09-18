#pragma once
#include <limits>

#include "augs/templates/type_matching_and_indexing.h"
#include "augs/templates/type_list.h"

template <class SizedType, class IndexType>
struct is_size_sufficient : std::bool_constant<std::numeric_limits<IndexType>::max() >= sizeof(SizedType) - 1> {

};

template <class T>
struct get_index_type_for_size_of {
	using type = find_matching_type_in_list<
		bind_types_t<is_size_sufficient, T>, 
		type_list<unsigned char, unsigned short, unsigned int>
	>;
};

template <class T>
using get_index_type_for_size_of_t = typename get_index_type_for_size_of<T>::type;
