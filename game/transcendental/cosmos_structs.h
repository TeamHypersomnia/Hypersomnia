#pragma once
#include "augs/misc/trivially_copyable_tuple.h"

#include "game/assets/animation_id.h"
#include "game/assets/particle_effect_id.h"
#include "game/assets/tile_layer_id.h"
#include "game/assets/sound_buffer_id.h"

#include "game/assets/animation.h"
#include "game/assets/particle_effect.h"
#include "game/assets/tile_layer.h"
#include "game/assets/spell.h"
#include "game/assets/physical_material.h"

#include "game/assets/assets_manager.h"

namespace augs {
	struct introspection_access;
}

class cosmos_metadata {
	// GEN INTROSPECTOR class cosmos_metadata
	friend class cosmos;
	friend struct augs::introspection_access;

	augs::delta delta;
	unsigned total_steps_passed = 0;

#if COSMOS_TRACKS_GUIDS
	entity_guid next_entity_guid = 1;
#endif
public:
	all_simulation_settings settings;
	// END GEN INTROSPECTOR
};

typedef put_all_components_into_t<augs::operations_on_all_components_mixin, cosmos> cosmos_base;

class cosmos_significant_state {
	// GEN INTROSPECTOR class cosmos_significant_state
	friend class cosmos;
	friend class cosmic_delta;
	friend class assets_manager;
	friend struct augs::introspection_access;

	assets_manager::tuple_of_all_logical_metas_of_assets logical_metas_of_assets;

	typename cosmos_base::aggregate_pool_type pool_for_aggregates;
	typename cosmos_base::component_pools_type pools_for_components;
public:
	cosmos_metadata meta;
	// END GEN INTROSPECTOR

	bool operator==(const cosmos_significant_state&) const;
	bool operator!=(const cosmos_significant_state&) const;
};

enum class subjects_iteration_flag {
	POSSIBLE_ITERATOR_INVALIDATION,

	COUNT
};

template <class T>
using find_logical_metas_container_t = find_type_with_key_type_in_list_t<T, assets_manager::tuple_of_all_logical_metas_of_assets>;

class cosmic_delta;
struct data_living_one_step;