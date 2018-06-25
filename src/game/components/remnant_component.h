#pragma once
#include "augs/math/declare_math.h"

namespace invariants {
	struct remnant {
		// GEN INTROSPECTOR struct invariants::remnant
		real32 lifetime_secs = 2.f;
		real32 start_shrinking_when_remaining_ms = 10.0f;
		particle_effect_input trace_particles;
		// END GEN INTROSPECTOR
	};
}


namespace components {
	struct remnant {
		// GEN INTROSPECTOR struct components::remnant
		real32 last_size_mult = 1.f;
		// END GEN INTROSPECTOR
	};
}
