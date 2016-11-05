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

		attenuation attenuations[4];
		attenuation wall_attenuations[4];
		value_variation position_variations[2];
	};
}