#pragma once
#include <cstdint>
#include "augs/math/declare_math.h"

namespace augs {
	/*
		Motion of a body integrated like in Box2D, with linear damping c:
		every step of dt seconds, first v *= a, where a = 1 / (1 + dt * c), then pos += dt * v.

		So after n steps, a body starting at speed v0 covers:

			dt * v0 * (a + a^2 + ... + a^n) = (v0 / c) * (1 - a^n)

		since a / (1 - a) = 1 / (dt * c).

		a^n is computed by repeated multiplication, not pow, so that it stays deterministic across platforms.
	*/

	inline real32 calc_damped_distance_mult(
		const real32 linear_damping,
		const real32 dt_secs,
		const uint32_t steps
	) {
		if (linear_damping <= 0.f) {
			return dt_secs * static_cast<real32>(steps);
		}

		const auto a = 1.f / (1.f + dt_secs * linear_damping);

		auto a_to_n = 1.f;

		for (uint32_t i = 0; i < steps; ++i) {
			a_to_n *= a;
		}

		return (1.f - a_to_n) / linear_damping;
	}

	inline real32 calc_damped_distance(
		const real32 initial_speed,
		const real32 linear_damping,
		const real32 dt_secs,
		const uint32_t steps
	) {
		return initial_speed * calc_damped_distance_mult(linear_damping, dt_secs, steps);
	}

	/*
		The inverse of calc_damped_distance.
	*/

	inline real32 calc_initial_speed_for_damped_distance(
		const real32 distance,
		const real32 linear_damping,
		const real32 dt_secs,
		const uint32_t steps
	) {
		const auto mult = calc_damped_distance_mult(linear_damping, dt_secs, steps);

		if (mult <= 0.f) {
			return 0.f;
		}

		return distance / mult;
	}
}
