#pragma once
#include "augs/misc/constant_size_vector.h"
#include "augs/templates/value_with_flag.h"

namespace invariants {
	struct movement_path {
		// GEN INTROSPECTOR struct invariants::movement_path
		real32 continuous_rotation_speed = 0.f;
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
