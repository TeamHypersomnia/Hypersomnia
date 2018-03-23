#pragma once
#include <array>
#include "augs/graphics/rgba.h"

struct randomization;

struct light_value_variation {
	// GEN INTROSPECTOR struct light_value_variation
	float min_variation = 0.f;
	float max_variation = 0.f;
	float change_speed = 0.f;
	// END GEN INTROSPECTOR

	void update_value(randomization&, float& val, const float dt_seconds) const;
};

struct light_attenuation {
	// GEN INTROSPECTOR struct light_attenuation
	float base_value = 0.f;
	light_value_variation variation;
	// END GEN INTROSPECTOR
};

namespace components {
	struct light {
		// GEN INTROSPECTOR struct components::light
		rgba color = white;
		// END GEN INTROSPECTOR
	};
}

namespace invariants {
	struct light {
		using implied_component = components::light;

		// GEN INTROSPECTOR struct invariants::light
		light_attenuation constant;
		light_attenuation linear;
		light_attenuation quadratic;
		light_attenuation max_distance;

		light_attenuation wall_constant;
		light_attenuation wall_linear;
		light_attenuation wall_quadratic;
		light_attenuation wall_max_distance;

		std::array<light_value_variation, 2> position_variations;
		// END GEN INTROSPECTOR

		light();
	};
}