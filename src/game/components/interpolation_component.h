#pragma once
#include "game/components/transform_component.h"

namespace components {
	struct interpolation {
		// GEN INTROSPECTOR struct components::interpolation
		components::transform place_of_birth;
		// END GEN INTROSPECTOR
	};
}

namespace invariants {
	struct interpolation {
		using implied_component = components::interpolation;

		// GEN INTROSPECTOR struct invariants::interpolation
		float base_exponent = 0.9f;
		// END GEN INTROSPECTOR
	};
}
