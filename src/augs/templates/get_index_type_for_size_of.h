#pragma once
#include <limits>

#include "augs/templates/predicate_templates.h"
#include "augs/templates/filter_types.h"
#include "augs/templates/type_list.h"

template <class SizedType, class IndexType>
using is_size_sufficient = std::bool_constant<std::numeric_limits<IndexType>::max() >= sizeof(SizedType) - 1>;

template <class T>
using get_index_type_for_size_of_t = find_matching_type_in_list<
	bind_types<is_size_sufficient, T>::template type,
	type_list<unsigned char, unsigned short, unsigned int>
>;
