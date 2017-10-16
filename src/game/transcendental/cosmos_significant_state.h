#pragma once
#include "augs/misc/constant_size_vector.h"
#include "augs/misc/pool/pool.h"
#include "game/transcendental/cosmos_metadata.h"
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
	friend struct augs::introspection_access;

	entity_pool_type entity_pool;
	dynamic_component_pools_type component_pools;
public:
	cosmos_metadata meta;
	// END GEN INTROSPECTOR

	void clear();

	/* TODO: Make comparisons somehow work with debug name pointers */
	/* These would eat too much space due to cosmos copies for modification */
#if !(DEBUG_TRACK_ENTITY_NAME && STATICALLY_ALLOCATE_ENTITIES_NUM)
	std::size_t get_first_mismatch_pos(const cosmos_significant_state&) const;

	bool operator==(const cosmos_significant_state&) const;
	bool operator!=(const cosmos_significant_state&) const;
#endif
};