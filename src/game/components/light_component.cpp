#include <cmath>
#include <limits>
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

legacy_attenuation light_falloff::calc_legacy(const real32 alpha) const {
	/*
		Least squares over the radius, weighted by the area each distance covers.
		The candidates: attenuations mixing constant, linear and quadratic terms, in units of the radius,
		times the brightness they have at the radius (strength) - the legacy model's own parameters.
		Both curves are compared as they reach the light texture: the physical one scaled by the color's alpha,
		the legacy one capped by it.
	*/

	legacy_attenuation result;

	if (radius <= 0.0f) {
		return result;
	}

	constexpr int num_samples = 24;
	constexpr int num_mixes = 21;
	constexpr int num_strengths = 40;

	std::array<real32, num_samples> xs;
	std::array<real32, num_samples> target;

	const auto core = calc_core();

	for (int i = 0; i < num_samples; ++i) {
		const auto x = (i + 0.5f) / num_samples;
		const auto core_x = x / core;
		const auto window = std::clamp(1.0f - x * x * x * x, 0.0f, 1.0f);

		xs[i] = x;
		target[i] = std::min(1.0f, alpha * intensity * window * window / (1.0f + core_x * core_x));
	}

	auto mix_terms = [](const real32 t) {
		return std::array<real32, 3> {
			std::max(0.0f, 1.0f - 2.0f * t),
			1.0f - std::abs(2.0f * t - 1.0f),
			std::max(0.0f, 2.0f * t - 1.0f)
		};
	};

	auto best_error = std::numeric_limits<real32>::max();
	auto best_terms = mix_terms(0.5f);
	auto best_strength = 30.0f;

	for (int m = 0; m < num_mixes; ++m) {
		const auto terms = mix_terms(static_cast<real32>(m) / (num_mixes - 1));
		const auto edge = terms[0] + terms[1] + terms[2];

		for (int k = 0; k < num_strengths; ++k) {
			const auto strength = std::round(std::pow(255.0f, static_cast<real32>(k) / (num_strengths - 1)));
			const auto cutoff_x = std::max(0.0f, 1.0f - 3.0f * strength / radius);

			auto error = 0.0f;

			for (int i = 0; i < num_samples; ++i) {
				const auto x = xs[i];
				const auto atten = terms[0] + terms[1] * x + terms[2] * x * x;

				auto legacy = atten > 0.0f ? (strength / 255.0f) * edge / atten : 1.0f;

				if (x > cutoff_x) {
					const auto u = (x - cutoff_x) / (1.0f - cutoff_x);
					legacy *= 0.5f * (1.0f - u * u);
				}

				const auto diff = std::min(alpha, legacy) - target[i];
				error += diff * diff * x;
			}

			if (error < best_error) {
				best_error = error;
				best_terms = terms;
				best_strength = strength;
			}
		}
	}

	/*
		From units of the radius to pixels, scaled like the editor's legacy lights:
		the attenuation at the radius gives exactly strength / 255.
	*/

	const auto edge = best_terms[0] + best_terms[1] + best_terms[2];
	const auto mult = 255.0f / (edge * best_strength);

	result.constant = best_terms[0] * mult * CONST_MULT;
	result.linear = best_terms[1] / radius * mult * LINEAR_MULT;
	result.quadratic = best_terms[2] / (radius * radius) * mult * QUADRATIC_MULT;
	result.trim_alpha = static_cast<rgba_channel>(best_strength);

	return result;
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
