#include "augs/misc/randomization.h"
#include "game/components/light_component.h"
#include "view/audiovisual_state/systems/legacy_light_mults.h"

namespace components {
	light::light() {
		wall_attenuation.constant = attenuation.constant = 0.4f * CONST_MULT;
		wall_attenuation.linear = attenuation.linear = 0.000005f * LINEAR_MULT;
		wall_attenuation.quadratic = attenuation.quadratic = 0.000055f * QUADRATIC_MULT;

		//wall_attenuation.constant *= 2.5;
		wall_attenuation.linear *= 2;
		wall_attenuation.quadratic *= 2;

		{
			variation.is_enabled = true;

			auto& v = variation.value;

			v.constant.magnitude = 0.05f * CONST_MULT;
			v.constant.change_speed = 0.8f / 10;

			v.linear.magnitude = 0.000005f* LINEAR_MULT;
			v.linear.change_speed = 0.0002f / 5* LINEAR_MULT;

			v.quadratic.magnitude = 0.00001f* QUADRATIC_MULT;
			v.quadratic.change_speed = 0.0003f / 12* QUADRATIC_MULT;
		}

		wall_variation = variation;

		position_variations.is_enabled = true;

		auto& p = position_variations.value;

		p[0].change_speed = 50;
		p[0].magnitude = 20;

		p[1].change_speed = 50;
		p[1].magnitude = 20;
	}

	vec2 light::calc_reach_trimmed() const {
		auto props = attenuation;

		if (variation.is_enabled) {
			//props.add_max(variation.value);
		}

		return props.calc_reach_trimmed();
	}

	vec2 light::calc_wall_reach_trimmed() const {
		auto props = wall_attenuation;

		if (wall_variation.is_enabled) {
			//props.add_max(wall_variation.value);
		}

		return props.calc_reach_trimmed();
	}

	vec2 light::calc_effective_reach() const {
		return std::max(calc_wall_reach_trimmed(), calc_reach_trimmed());
	}
}

using real_type = real32;

FORCE_INLINE real_type light_max_pixel_distance(real_type a, real_type b, real_type c, real_type d) {
	/* 
		Derived from:
		http://www.wolframalpha.com/input/?i=1%2F(a+%2B+b+*+x+%2B+c+*+x+*+x)+%3E%3D+(1+%2F+255),+a+%3E+0,+b+%3E+0,+c+%3E+0,+solve+x

		(query: 1/(a + b * x + c * x * x) >= (1 / 255), a > 0, b > 0, c > 0, solve x)

		Despite knowing next to none of what happens here, I will forever respect mathematics henceforth.

		return (c * sqrt( (-4*a*c+b*b+1020*c) / (c*c) ) - b)/(2*c);
	*/

	auto at_least_epsilon = [](auto& v) {
		v = std::max(v, real_type(0.0000000001));
	};

	at_least_epsilon(a);
	at_least_epsilon(b);
	at_least_epsilon(c);
	at_least_epsilon(d);

	return (c * repro::sqrt( (-4*a*c*d+b*b*d+1020*c) / c/c/d ) - b)/2/c;
};

real32 attenuation_properties::calc_reach() const {
	return static_cast<real32>(light_max_pixel_distance(
		constant / CONST_MULT,
		linear / LINEAR_MULT,
		quadratic / QUADRATIC_MULT,
		static_cast<real_type>(std::max(rgba_channel(1), trim_alpha))
	));
}

vec2 attenuation_properties::calc_reach_trimmed() const {
	auto result = vec2::square(calc_reach() * 2);

	if (trim_reach.is_enabled) {
		result.lessen(trim_reach.value);
	}

	return result;
}

void light_value_variation::update_value(randomization& rng, atten_t& val, const float dt_seconds) const {
	const auto speed_dt = rng.randval(-change_speed, change_speed);
	const auto total_dt = dt_seconds * speed_dt;
	val += total_dt;
	const auto h = magnitude / 2;
	val = std::clamp(val, -h, h);
}
