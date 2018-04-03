#include "augs/misc/randomization.h"
#include "game/components/light_component.h"

#define CONST_MULT 100
#define LINEAR_MULT 10000
#define QUADRATIC_MULT 10000000

namespace invariants {
	light::light() {
		constant.base_value = 0.4f * CONST_MULT;
		linear.base_value = 0.000005f * LINEAR_MULT;
		quadratic.base_value = 0.000055f * QUADRATIC_MULT;
		max_distance.base_value = 4500.f;

		wall_constant.base_value = 0.4f * CONST_MULT;
		wall_linear.base_value = 0.000017f * LINEAR_MULT;
		wall_quadratic.base_value = 0.000037f* QUADRATIC_MULT;
		wall_max_distance.base_value = 4000.f;

		constant.variation.min_variation = -0.05f * CONST_MULT;
		constant.variation.max_variation = 0.0f;
		constant.variation.change_speed = 0.8f / 10;

		linear.variation.min_variation = -0.000005f* LINEAR_MULT;
		linear.variation.max_variation = 0.00000f* LINEAR_MULT;
		linear.variation.change_speed = 0.0002f / 5* LINEAR_MULT;

		quadratic.variation.min_variation = -0.00001f* QUADRATIC_MULT;
		quadratic.variation.max_variation = 0.00000f* QUADRATIC_MULT;
		quadratic.variation.change_speed = 0.0003f / 12* QUADRATIC_MULT;

		wall_constant.variation = constant.variation;
		wall_linear.variation = linear.variation;
		wall_quadratic.variation = quadratic.variation;

		position_variations[0].change_speed = 50;
		position_variations[0].min_variation = -10;
		position_variations[0].max_variation = 10;

		position_variations[1].change_speed = 50;
		position_variations[1].min_variation = -10;
		position_variations[1].max_variation = 10;

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

	float light::get_max_distance() const {
		return std::max(max_distance.get_max(), wall_max_distance.get_max());
	}
}

void light_value_variation::update_value(randomization& rng, float& val, const float dt_seconds) const {
	val += dt_seconds * rng.randval(-change_speed, change_speed);

	if (val < min_variation) {
		val = min_variation;
	}

	if (val > max_variation) {
		val = max_variation;
	}
}
