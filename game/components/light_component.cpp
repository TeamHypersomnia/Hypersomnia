#include "light_component.h"
#include "augs/misc/randomization.h"

namespace components {
	light::light() {
		constant.base_value = 0.4f;
		linear.base_value = 0.000011f;
		quadratic.base_value = 0.000055f;
		max_distance.base_value = 2000.f;

		wall_constant.base_value = 0.4f;
		wall_linear.base_value = 0.000017f;
		wall_quadratic.base_value = 0.000037f;
		wall_max_distance.base_value = 2000.f;

		constant.variation.min_value = -0.05;
		constant.variation.max_value = 0.0;
		constant.variation.change_speed = 0.8 / 10;

		linear.variation.min_value = -0.000005;
		linear.variation.max_value = 0.00000;
		linear.variation.change_speed = 0.0002 / 5;

		quadratic.variation.min_value = -0.00001;
		quadratic.variation.max_value = 0.00000;
		quadratic.variation.change_speed = 0.0003 / 12;

		wall_constant.variation = constant.variation;
		wall_linear.variation = linear.variation;
		wall_quadratic.variation = quadratic.variation;

		position_variations[0].change_speed = 50;
		position_variations[0].min_value = -10;
		position_variations[0].max_value = 10;

		position_variations[1].change_speed = 50;
		position_variations[1].min_value = -10;
		position_variations[1].max_value = 10;

		/*
		
		constant.base_value = 0.7f;
		linear.base_value = 0.00002f - 0.00001f;
		quadratic.base_value = 0.00007f - 0.00005f;
		max_distance.base_value = 2000.f;

		wall_constant.base_value = 0.7f;
		wall_linear.base_value = 0.00002f - 0.00001f;
		wall_quadratic.base_value = 0.000037f - 0.00005f;
		wall_max_distance.base_value = 2000.f;
		*/
	}

	void light::value_variation::update_value(randomization& rng, float& val, const float dt_seconds) const {
		val += dt_seconds * rng.randval(-change_speed, change_speed);

		if (val < min_value) {
			val = min_value;
		}

		if (val > max_value) {
			val = max_value;
		}
	}
}
