#pragma once
#include "augs/misc/constant_size_vector.h"
#include "augs/misc/pool/pool.h"
#include "game/transcendental/cosmos_common_state.h"
#include "game/transcendental/cosmic_types.h"

using cosmos_base = component_list_t<augs::operations_on_all_components_mixin, cosmos>;

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

class cosmos_significant_state {
	// GEN INTROSPECTOR class cosmos_significant_state
	friend class cosmos;
	friend class cosmic_delta;
	friend augs::introspection_access;

	entity_pool_type entity_pool;
	dynamic_component_pools_type component_pools;
public:
	cosmos_common_state common;
	// END GEN INTROSPECTOR

	void clear();
};