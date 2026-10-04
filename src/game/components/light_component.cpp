#include <cmath>
#include "augs/misc/randomization.h"
#include "game/components/light_component.h"
#include "view/audiovisual_state/systems/legacy_light_mults.h"

real32 light_falloff::calc_core() const {
	/*
		0.08 at falloff 1 - a sharp hotspot, 1 at falloff 0 - nearly flat.
		At the default 0.5 the light at half its radius matches the linear falloff of the legacy model.
	*/

	return std::pow(0.08f, std::clamp(falloff, 0.0f, 1.0f));
}

real32 light_falloff::calc_legacy_constant() const {
	return std::max(0.0f, 1.0f - 2.0f * falloff);
}

real32 light_falloff::calc_legacy_linear() const {
	return 1.0f - std::abs(2.0f * falloff - 1.0f);
}

real32 light_falloff::calc_legacy_quadratic() const {
	return std::max(0.0f, 2.0f * falloff - 1.0f);
}

rgba_channel light_falloff::calc_legacy_strength() const {
	return static_cast<rgba_channel>(std::clamp(intensity * 30.0f, 1.0f, 255.0f));
}

namespace components {
	light::light() {
		wall_falloff.radius = falloff.radius / 1.5f;

		wall_attenuation.constant = attenuation.constant = 0.4f * CONST_MULT;
		wall_attenuation.linear = attenuation.linear = 0.000005f * LINEAR_MULT;
		wall_attenuation.quadratic = attenuation.quadratic = 0.000055f * QUADRATIC_MULT;

		wall_attenuation.linear *= 2;
		wall_attenuation.quadratic *= 2;

		variation.is_enabled = true;
		variation.value.magnitude = 0.1f;
		variation.value.change_speed = 0.2f;

		wall_variation = variation;
	}

	vec2 light::calc_reach() const {
		return falloff.calc_reach();
	}

	vec2 light::calc_wall_reach() const {
		return wall_falloff.calc_reach();
	}

	vec2 light::calc_effective_reach() const {
		return std::max(calc_wall_reach(), calc_reach());
	}
}

FORCE_INLINE real32 light_max_pixel_distance(real32 a, real32 b, real32 c, real32 d) {
	/* 
		Derived from:
		http://www.wolframalpha.com/input/?i=1%2F(a+%2B+b+*+x+%2B+c+*+x+*+x)+%3E%3D+(1+%2F+255),+a+%3E+0,+b+%3E+0,+c+%3E+0,+solve+x

		(query: 1/(a + b * x + c * x * x) >= (1 / 255), a > 0, b > 0, c > 0, solve x)
	*/

	auto at_least_epsilon = [](auto& v) {
		v = std::max(v, real32(0.0000000001));
	};

	at_least_epsilon(a);
	at_least_epsilon(b);
	at_least_epsilon(c);
	at_least_epsilon(d);

	return (c * repro::sqrt( (-4*a*c*d+b*b*d+1020*c) / c/c/d ) - b)/2/c;
};

real32 legacy_attenuation::calc_reach() const {
	return light_max_pixel_distance(
		constant / CONST_MULT,
		linear / LINEAR_MULT,
		quadratic / QUADRATIC_MULT,
		static_cast<real32>(std::max(rgba_channel(1), trim_alpha))
	);
}

void light_value_variation::update_value(randomization& rng, real32& val, const float dt_seconds) const {
	const auto speed_dt = rng.randval(-change_speed, change_speed);
	const auto total_dt = dt_seconds * speed_dt;
	val += total_dt;
	const auto h = magnitude / 2;
	val = std::clamp(val, -h, h);
}
