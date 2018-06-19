#pragma once
#include "augs/misc/constant_size_vector.h"
#include "augs/templates/value_with_flag.h"

struct square_bounded_movement {
	// GEN INTROSPECTOR struct square_bounded_movement
	vec2 square_size;
	unsigned seed_offset = 0u;
	// END GEN INTROSPECTOR
};

namespace invariants {
	struct movement_path {
		// GEN INTROSPECTOR struct invariants::movement_path
		real32 continuous_rotation_speed = 0.f;
		augs::value_with_flag<square_bounded_movement> square_bounded;
		// END GEN INTROSPECTOR
	};
}

namespace components {
	struct movement_path {
		// GEN INTROSPECTOR struct components::movement_path
		real32 path_time = 0.f;
		// END GEN INTROSPECTOR
	};
}
