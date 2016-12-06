#pragma once
#include <vector>
#include "augs/math/vec2.h"
#include "particle_effect_response_component.h"
#include "game/resources/particle_effect.h"
#include "transform_component.h"

#include "game/detail/state_for_drawing_camera.h"
#include "augs/misc/stepped_timing.h"

#include "augs/padding_byte.h"

namespace components {
	struct particles_existence {
		struct effect_input {
			assets::particle_effect_id effect = assets::particle_effect_id::INVALID;
			resources::particle_effect_modifier modifier;

			float randomize_position_within_radius = 0.f;
			augs::minmax<float> single_displacement_duration_ms = augs::minmax<float>(0.f, 0.f);
		} input;

		vec2 current_displacement;
		augs::stepped_timestamp time_of_last_displacement;
		float current_displacement_duration_bound_ms = 0.f;

		augs::stepped_timestamp time_of_birth;
		unsigned rng_seed = 0u;
		unsigned max_lifetime_in_steps = 0u;

		float distribute_within_segment_of_length = 0.f;

		bool operator==(const particles_existence&) const;
		bool operator!=(const particles_existence&) const;
	};
}