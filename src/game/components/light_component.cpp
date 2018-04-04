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

		wall_constant.base_value = 0.4f * CONST_MULT;
		wall_linear.base_value = 0.000017f * LINEAR_MULT;
		wall_quadratic.base_value = 0.000037f* QUADRATIC_MULT;

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
	}

	float light::get_max_distance() const {
		float considered_max = max_distance;
	   
		if (considered_max < 0.f) {
			considered_max = calc_pixel_distance();
		}

		float considered_wall_max = wall_max_distance;

		if (considered_wall_max < 0.f) {
			considered_wall_max = calc_pixel_wall_distance();
		}

		return std::max(considered_max, considered_wall_max);
	}

	inline double light_max_pixel_distance(const double a, const double b, const double c) {
		/* 
			Derived from:
			http://www.wolframalpha.com/input/?i=1%2F(a+%2B+b+*+x+%2B+c+*+x+*+x)+%3E%3D+(1+%2F+255),+a+%3E+0,+b+%3E+0,+c+%3E+0,+solve+x

			(query: 1/(a + b * x + c * x * x) >= (1 / 255), a > 0, b > 0, c > 0, solve x)

			Despite knowing next to none of what happens here, I shall forever respect mathematics henceforth.
		*/

		return (c * sqrt( (-4*a*c+b*b+1020*c) / (c*c) ) - b)/(2*c);
	};


	float light::calc_pixel_distance() const {
		return static_cast<float>(light_max_pixel_distance(
			constant.get_max() / CONST_MULT,
			linear.get_max() / LINEAR_MULT,
			quadratic.get_max() / QUADRATIC_MULT
		));
	}

	float light::calc_pixel_wall_distance() const {
		return static_cast<float>(light_max_pixel_distance(
			wall_constant.get_max() / CONST_MULT,
			wall_linear.get_max() / LINEAR_MULT,
			wall_quadratic.get_max() / QUADRATIC_MULT
		));
	}

	float light::calc_max_pixel_distance() const {
		return std::max(calc_pixel_distance(), calc_pixel_wall_distance());
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
