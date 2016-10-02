#pragma once
#include "augs/misc/pool_id.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include <vector>

#include "game/components/transform_component.h"

class interpolation_system {
public:
	struct cache {
		components::transform recorded_place_of_birth;
		components::transform interpolated_transform;
		unsigned recorded_version = 0;
	};

	std::vector<cache> per_entity_cache;
	float interpolation_speed = 525.f;

	void integrate_interpolated_transforms(const cosmos&, float seconds);

	void construct(const const_entity_handle);
	void destruct(const const_entity_handle);

	components::transform& get_interpolated(const const_entity_handle);
	void reserve_caches_for_entities(const size_t);
	void write_current_to_interpolated(const const_entity_handle);
};