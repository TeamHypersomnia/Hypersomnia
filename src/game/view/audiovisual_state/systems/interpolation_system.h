#pragma once
#include "augs/misc/pooled_object_id.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include <vector>

#include "game/components/transform_component.h"
#include "augs/misc/delta.h"

struct interpolation_settings;

class interpolation_system {
	bool enabled = true;
	void set_interpolation_enabled(const bool);

public:
	struct cache {
		components::transform recorded_place_of_birth;
		components::transform interpolated_transform;
		unsigned recorded_version = 0;
		float rotational_slowdown_multiplier = 1.f;
		float positional_slowdown_multiplier = 1.f;
	};

	std::vector<cache> per_entity_cache;

	void integrate_interpolated_transforms(
		const interpolation_settings&,
		const cosmos&,
		const augs::delta delta, 
		const augs::delta fixed_delta_for_slowdowns
	);

	components::transform get_interpolated(const const_entity_handle) const;
	components::transform& get_interpolated(const const_entity_handle);

	void reserve_caches_for_entities(const size_t);

	cache& get_cache_of(const entity_id);

	void set_updated_interpolated_transform(
		const const_entity_handle subject,
		const components::transform updated_value
	);
};