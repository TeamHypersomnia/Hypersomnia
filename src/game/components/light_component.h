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

	auto get_max() const {
		return base_value + variation.max_variation;
	}
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
		// GEN INTROSPECTOR struct invariants::light
		light_attenuation constant;
		light_attenuation linear;
		light_attenuation quadratic;
		float max_distance = -1.f;

		light_attenuation wall_constant;
		light_attenuation wall_linear;
		light_attenuation wall_quadratic;
		float wall_max_distance = -1.f;

		std::array<light_value_variation, 2> position_variations;
		// END GEN INTROSPECTOR

		light();

		float get_max_distance() const;

		float calc_pixel_distance() const;
		float calc_pixel_wall_distance() const;

		float calc_max_pixel_distance() const;
	};
}