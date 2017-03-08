#pragma once
#include <vector>
#include "augs/math/vec2.h"
#include "particle_effect_response_component.h"
#include "game/resources/particle_effect.h"
#include "transform_component.h"

#include "augs/misc/stepped_timing.h"

#include "augs/padding_byte.h"

#include "game/transcendental/entity_handle_declaration.h"

namespace components {
	struct particles_existence {
		struct effect_input {
			// GEN INTROSPECTOR struct components::particles_existence::effect_input
			assets::particle_effect_id effect = assets::particle_effect_id::INVALID;
			bool delete_entity_after_effect_lifetime = true;
			padding_byte pad;

			resources::particle_effect_modifier modifier;

			float displace_source_position_within_radius = 0.f;
			augs::minmax<float> single_displacement_duration_ms = augs::minmax<float>(0.f, 0.f);
			// END GEN INTROSPECTOR
		};
		
		// GEN INTROSPECTOR struct components::particles_existence
		effect_input input;

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