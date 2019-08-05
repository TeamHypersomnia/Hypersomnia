#pragma once
#include "augs/misc/timing/delta.h"
#include "augs/misc/randomization.h"

#include "game/cosmos/entity_id.h"
#include "game/cosmos/entity_handle_declaration.h"
#include "game/cosmos/step_declaration.h"

#include "game/components/wandering_pixels_component.h"

#include "view/audiovisual_state/systems/audiovisual_cache_common.h"
#include "view/audiovisual_state/systems/linear_cache_map.h"

class visible_entities;

class interpolation_system;
struct particles_emission;
struct randomization;

class wandering_pixels_system {
public:
	struct particle {
		vec2 pos;
		vec2 current_direction = vec2(1, 0);
		float current_velocity = 20.f;
		float direction_ms_left = 0.f;
		float current_lifetime_ms = 0.f;
	};

	struct cache {
		unsigned recorded_particle_count = 0;
		xywh recorded_reach;

		std::vector<particle> particles;

		bool is_set() const {
			return recorded_particle_count > 0;
		}
	};

	double global_time_seconds = 0.0;

	linear_cache_map<cache, entity_types_having_all_of<components::wandering_pixels>> per_entity_cache;

	void clear() {
		per_entity_cache.clear();
	}

	template <class E>
	cache& get_cache(const E& id);

	template <class E>
	const cache* find_cache(const E& id) const;

	template <class E>
	void advance_for(
		const E& subject,
		const augs::delta dt
	);

	void reserve_caches_for_entities(const std::size_t n) {
		per_entity_cache.reserve(n);
	}
};