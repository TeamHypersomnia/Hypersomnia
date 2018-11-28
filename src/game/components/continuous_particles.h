#pragma once
#include "augs/math/declare_math.h"
#include "augs/templates/maybe.h"
#include "augs/misc/minmax.h"
#include "game/detail/view_input/particle_effect_input.h"

struct stream_wandering {
	// GEN INTROSPECTOR struct stream_wandering
	real32 additional_radius = 10.f;
	augs::minmax<real32> duration_ms = augs::minmax<real32>(200.f, 2000.f);
	// END GEN INTROSPECTOR
};

struct stream_wandering_state {
	// GEN INTROSPECTOR struct stream_wandering_state
	augs::stepped_timestamp when_last;
	real32 current_duration_ms = 0.f;
	vec2 current;
	// END GEN INTROSPECTOR
};

namespace invariants {
	struct continuous_particles {
		// GEN INTROSPECTOR struct invariants::continuous_particles
		particle_effect_input effect;
		augs::maybe<stream_wandering> wandering;
		real32 max_lifetime_ms = -1.f;
		// END GEN INTROSPECTOR
	};
}

namespace components {
	struct continuous_particles {
		// GEN INTROSPECTOR struct components::continuous_particles
		stream_wandering_state wandering_state;
		// END GEN INTROSPECTOR
	};
}
