#pragma once
#include "augs/misc/constant_size_vector.h"
#include "augs/misc/pool/pool.h"

#include "game/transcendental/cosmic_types.h"
#include "game/transcendental/cosmos_meta.h"

using dynamic_component_pools_type = 
	replace_list_type_t<
		transform_types_in_list_t<
			cosmic_entity::dynamic_components_list,
			cosmic_object_pool
		>, 
		std::tuple
	>
;

using entity_pool_type = cosmic_object_pool<cosmic_entity>;

struct cosmos_solvable_significant {
	// GEN INTROSPECTOR struct cosmos_solvable_significant
	entity_pool_type entity_pool;
	dynamic_component_pools_type component_pools;

	cosmos_meta meta;
	// END GEN INTROSPECTOR

	void clear();
};