#pragma once
#include "augs/graphics/pixel.h"

namespace components {
	struct light {
		struct value_variation {
			float min_value = 0.f;
			float max_value = 0.f;
			float change_speed = 0.f;
		};

		struct attenuation {
			float base_value = 0.f;
			value_variation variation;
		};

		augs::rgba color;

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