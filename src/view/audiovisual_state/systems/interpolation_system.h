#pragma once
#include <vector>

#include "augs/misc/timing/delta.h"
#include "augs/misc/pool/pooled_object_id.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "game/components/transform_component.h"

struct interpolation_settings;

class interpolation_system {
	bool enabled = true;
	void set_interpolation_enabled(const bool);

public:
	struct cache {
		components::transform recorded_place_of_birth;
		components::transform interpolated_transform;
		decltype(entity_id::version) recorded_version = entity_id().version;
		float rotational_slowdown_multiplier = 1.f;
		float positional_slowdown_multiplier = 1.f;

		bool is_constructed() const {
			return recorded_version != entity_id().version;
		}
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
	void clear();

	cache& get_cache_of(const entity_id);

	void set_updated_interpolated_transform(
		const const_entity_handle subject,
		const components::transform updated_value
	);
};