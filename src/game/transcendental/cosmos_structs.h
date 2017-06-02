#pragma once
#include "augs/misc/stepped_timing.h"
#include "augs/misc/trivially_copyable_tuple.h"

#include "game/transcendental/cosmos_global_state.h"

namespace augs {
	struct introspection_access;
}

class cosmos_metadata {
	// GEN INTROSPECTOR class cosmos_metadata
	friend class cosmos;
	friend struct augs::introspection_access;

	augs::delta delta;
	augs::stepped_timestamp now = 0;

#if COSMOS_TRACKS_GUIDS
	entity_guid next_entity_guid = 1;
#endif
public:
	cosmos_global_state global;
	// END GEN INTROSPECTOR
};

typedef put_all_components_into_t<augs::operations_on_all_components_mixin, cosmos> cosmos_base;

class cosmos_significant_state {
	// GEN INTROSPECTOR class cosmos_significant_state
	friend class cosmos;
	friend class cosmic_delta;
	friend struct augs::introspection_access;

	typename cosmos_base::aggregate_pool_type pool_for_aggregates;
	typename cosmos_base::component_pools_type pools_for_components;
public:
	cosmos_metadata meta;
	// END GEN INTROSPECTOR

	std::size_t get_first_mismatch_pos(const cosmos_significant_state&) const;

	bool operator==(const cosmos_significant_state&) const;
	bool operator!=(const cosmos_significant_state&) const;
};

enum class subjects_iteration_flag {
	POSSIBLE_ITERATOR_INVALIDATION,

	COUNT
};

class cosmic_delta;
struct data_living_one_step;