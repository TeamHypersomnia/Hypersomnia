#pragma once
#include <array>
#include "augs/graphics/pixel.h"

struct randomization;

namespace components {
	struct light {
		struct value_variation {
			float min_value = 0.f;
			float max_value = 0.f;
			float change_speed = 0.f;

			void update_value(randomization&, float& val, const float dt_seconds) const;
		};

		struct attenuation {
			float base_value = 0.f;
			value_variation variation;
		};

		rgba color;

		light();

		attenuation constant;
		attenuation linear;
		attenuation quadratic;
		attenuation max_distance;
		
		attenuation wall_constant;
		attenuation wall_linear;
		attenuation wall_quadratic;
		attenuation wall_max_distance;

		value_variation position_variations[2];
	};
}