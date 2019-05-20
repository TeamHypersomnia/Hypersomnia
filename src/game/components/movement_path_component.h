#pragma once
#include "augs/misc/enum/enum_array.h"
#include "augs/misc/constant_size_vector.h"
#include "augs/templates/maybe.h"
#include "game/enums/startle_type.h"
#include "game/cosmos/entity_flavour_id.h"
#include "game/detail/view_input/particle_effect_input.h"

struct fish_movement_def {
	// GEN INTROSPECTOR struct fish_movement_def
	real32 sine_speed_boost = 100.f;
	real32 base_speed = 80.f;
	real32 base_bubble_interval_ms = 1000.f;
	unsigned seed_offset = 0u;
	particle_effect_input bubble_effect;

	real32 sine_wandering_amplitude = 10.f;
	real32 sine_wandering_period = 50.f;
	// END GEN INTROSPECTOR
};

namespace invariants {
	struct movement_path {
		// GEN INTROSPECTOR struct invariants::movement_path
		real32 continuous_rotation_speed = 0.f;
		augs::maybe<fish_movement_def> fish_movement;
		// END GEN INTROSPECTOR
	};
}

namespace components {
	struct movement_path {
		// GEN INTROSPECTOR struct components::movement_path
		signi_entity_id origin;
		real32 path_time = 0.f;
		real32 last_speed = 0.f;
		augs::enum_array<vec2, startle_type> startle;
		real32 next_bubble_in_ms = -1.f;
		// END GEN INTROSPECTOR

		void add_startle(const startle_type type, const vec2 amount) {
			startle[type] += amount;
		}
	};
}
