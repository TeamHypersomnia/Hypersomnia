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

struct cosmos_metadata {
	// GEN INTROSPECTOR struct cosmos_metadata
	friend class cosmos;

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

struct cosmos_significant_state {
	// GEN INTROSPECTOR struct cosmos_significant_state
	cosmos_metadata meta;
	assets_manager::tuple_of_all_logical_metas_of_assets logical_metas_of_assets;

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

template <class T>
using find_flyweights_container_t = find_type_with_key_type_in_list_t<T, assets_manager::tuple_of_all_logical_metas_of_assets>;

class cosmic_delta;
struct data_living_one_step;