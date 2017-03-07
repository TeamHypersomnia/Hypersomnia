#pragma once
#include <array>
#include "augs/graphics/pixel.h"

#include "augs/misc/randomization.h"

namespace components {
	struct light {
		struct value_variation {
			// GEN INTROSPECTOR components::light::value_variation
			float min_value = 0.f;
			float max_value = 0.f;
			float change_speed = 0.f;
			// END GEN INTROSPECTOR

			void update_value(randomization&, float& val, const float dt_seconds) const;
		};

		struct attenuation {
			// GEN INTROSPECTOR components::light::attenuation
			float base_value = 0.f;
			value_variation variation;
			// END GEN INTROSPECTOR
		};

		// GEN INTROSPECTOR components::light
		rgba color;

		attenuation constant;
		attenuation linear;
		attenuation quadratic;
		attenuation max_distance;
		
		attenuation wall_constant;
		attenuation wall_linear;
		attenuation wall_quadratic;
		attenuation wall_max_distance;

		std::array<value_variation, 2> position_variations;
		// END GEN INTROSPECTOR

		light();
	};
}