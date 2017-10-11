#pragma once
#include <vector>

#include "augs/pad_bytes.h"

#include "augs/math/vec2.h"
#include "augs/misc/timing/stepped_timing.h"
#include "augs/misc/minmax.h"

#include "game/assets/ids/particle_effect_id.h"

#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/step_declaration.h"

#include "game/components/transform_component.h"

#include "game/detail/view_input/particle_effect_input.h"

struct particles_existence_input {
	// GEN INTROSPECTOR struct particles_existence_input
	particle_effect_input effect;
	bool delete_entity_after_effect_lifetime = true;
	pad_bytes<3> pad;

	float displace_source_position_within_radius = 0.f;
	augs::minmax<float> single_displacement_duration_ms = augs::minmax<float>(0.f, 0.f);
	// END GEN INTROSPECTOR

	entity_handle create_particle_effect_entity(
		const logic_step,
		const components::transform place_of_birth,
		const entity_id chased_subject
	) const;

	void create_particle_effect_components(
		components::transform& out_transform,
		components::particles_existence& out_existence,
		components::position_copying& out_copying,
		const logic_step,
		const components::transform place_of_birth,
		const entity_id chased_subject
	) const;
};

namespace components {
	struct particles_existence {
		// GEN INTROSPECTOR struct components::particles_existence
		particles_existence_input input;

		vec2 current_displacement;
		augs::stepped_timestamp time_of_last_displacement;
		float current_displacement_duration_bound_ms = 0.f;

		augs::stepped_timestamp time_of_birth;
		unsigned max_lifetime_in_steps = 0u;

		float distribute_within_segment_of_length = 0.f;
		// END GEN INTROSPECTOR

		static bool is_activated(const const_entity_handle);
		static void activate(const entity_handle);
		static void deactivate(const entity_handle);

		bool operator==(const particles_existence&) const;
		bool operator!=(const particles_existence&) const;
	};
}