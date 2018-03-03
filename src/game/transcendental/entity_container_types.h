#pragma once
#include <tuple>

#include "augs/templates/type_mod_templates.h"
#include "augs/templates/transform_types.h"

template <class E>
struct entity_solvable;

template <class T>
using make_entity_vector = std::vector<entity_solvable<T>>;

using all_entity_vectors = 
	replace_list_type_t<
		transform_types_in_list_t<
			all_entity_types,
			make_entity_vector
		>,
		std::tuple
	>
;

