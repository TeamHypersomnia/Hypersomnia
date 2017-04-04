#pragma once
#include "game/assets/animation_id.h"
#include "game/flyweights/animation.h"

class cosmic_delta;
struct data_living_one_step;

typedef put_all_components_into_t<augs::operations_on_all_components_mixin, cosmos> cosmos_base;

struct cosmos_flyweights_state {
	// GEN INTROSPECTOR struct cosmos_flyweights_state
	augs::enum_associative_array<spell_type, spell_data> spells;
	augs::enum_associative_array<assets::animation_id, flyweights::animation> animations;
	collision_sound_matrix_type collision_sound_matrix;
	// END GEN INTROSPECTOR
};

class cosmos_metadata {
	// GEN INTROSPECTOR class cosmos_metadata
	friend class cosmos;

	augs::delta delta;
	unsigned total_steps_passed = 0;

#if COSMOS_TRACKS_GUIDS
	entity_guid next_entity_guid = 1;
#endif
public:
	all_simulation_settings settings;

	cosmos_flyweights_state flyweights;
	// END GEN INTROSPECTOR
};

struct cosmos_significant_state {
	// GEN INTROSPECTOR struct cosmos_significant_state
	cosmos_metadata meta;

	typename cosmos_base::aggregate_pool_type pool_for_aggregates;
	typename cosmos_base::component_pools_type pools_for_components;
	// END GEN INTROSPECTOR

	bool operator==(const cosmos_significant_state&) const;
	bool operator!=(const cosmos_significant_state&) const;
};

enum class subjects_iteration_flag {
	POSSIBLE_ITERATOR_INVALIDATION,

	COUNT
};
