#pragma once
#include <vector>
#include "augs/math/vec2.h"
#include "particle_effect_response_component.h"
#include "game/resources/particle_effect.h"
#include "transform_component.h"

#include "game/detail/state_for_drawing_camera.h"
#include "augs/misc/stepped_timing.h"

#include "augs/padding_byte.h"

class particles_simulation_system;

namespace components {
	struct particles_existence {
		struct drawing_input : vertex_triangle_buffer_reference {
			using vertex_triangle_buffer_reference::vertex_triangle_buffer_reference;

			components::transform renderable_transform;
			components::transform camera_transform;
			vec2 visible_world_area;
			augs::rgba colorize;
			bool use_neon_map = false;
		};

		augs::stepped_timestamp time_of_birth;
		unsigned rng_seed = 0u;
		unsigned max_lifetime_in_steps = 0u;

		assets::particle_effect_id effect = assets::particle_effect_id::INVALID;
		resources::particle_effect_modifier modifier;

		void draw(const drawing_input&) const;
	private:
		friend class particles_simulation_system;
	};
}