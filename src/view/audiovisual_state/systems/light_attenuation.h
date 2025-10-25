#pragma once
#include "augs/math/vec2.h"
#include "augs/graphics/rgba.h"

/*
	Shared attenuation calculation logic used by both editor and runtime systems.
*/

inline real32 calc_attenuation_mult_for_requested_radius(
	const real32 constant,
	const real32 linear,
	const real32 quadratic,
	const real32 radius,
	const rgba_channel strength
) {
	const auto atten_at_edge =
		constant +
		linear * radius +
		quadratic * radius * radius
	;

	if (atten_at_edge == 0.0f) {
		return 1.0f;
	}

	/*
		Strength is just another name for cutoff alpha.
	*/

	const auto cutoff_alpha = strength;
	return 255.f / (atten_at_edge * float(std::max(rgba_channel(1), cutoff_alpha)));
}
