#pragma once
#include <tuple>

#include "augs/templates/type_mod_templates.h"
#include "augs/templates/transform_types.h"

template <class E>
struct entity_solvable;

template <template <class> class Mod>
using per_entity_type = 
	replace_list_type_t<
		transform_types_in_list_t<
			all_entity_types,
			Mod
		>,
		std::tuple
	>
;
