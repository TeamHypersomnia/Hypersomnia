#pragma once
#include "augs/misc/constant_size_vector.h"
#include "augs/templates/value_with_flag.h"

struct rect_bounded_movement {
	// GEN INTROSPECTOR struct rect_bounded_movement
	vec2 rect_size;
	real32 sine_speed_boost = 100.f;
	real32 base_speed = 80.f;
	unsigned seed_offset = 0u;
	// END GEN INTROSPECTOR
};

namespace invariants {
	struct movement_path {
		// GEN INTROSPECTOR struct invariants::movement_path
		real32 continuous_rotation_speed = 0.f;
		augs::value_with_flag<rect_bounded_movement> rect_bounded;
		// END GEN INTROSPECTOR
	};
}

namespace components {
	struct movement_path {
		// GEN INTROSPECTOR struct components::movement_path
		transformr origin;
		real32 path_time = 0.f;
		real32 last_speed = 0.f;
		// END GEN INTROSPECTOR
	};
}
