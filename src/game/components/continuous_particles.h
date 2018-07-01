#pragma once
#include "augs/math/declare_math.h"
#include "augs/templates/value_with_flag.h"
#include "augs/misc/minmax.h"
#include "game/detail/view_input/particle_effect_input.h"

struct stream_displacement {
	// GEN INTROSPECTOR struct stream_displacement
	real32 radius = 100.f;
	augs::minmax<real32> duration_ms;
	// END GEN INTROSPECTOR
};

struct stream_displacement_state {
	// GEN INTROSPECTOR struct stream_displacement_state
	augs::stepped_timestamp when_last;
	real32 current_duration_ms = 0.f;
	vec2 current;
	// END GEN INTROSPECTOR
};

namespace invariants {
	struct continuous_particles {
		// GEN INTROSPECTOR struct invariants::continuous_particles
		particle_effect_input effect;
		augs::value_with_flag<stream_displacement> displacement;
		real32 max_lifetime_ms = -1.f;
		// END GEN INTROSPECTOR
	};
}

namespace components {
	struct continuous_particles {
		// GEN INTROSPECTOR struct components::continuous_particles
		stream_displacement_state displacement_state;
		// END GEN INTROSPECTOR
	};
}
