#pragma once
#include "augs/graphics/rgba.h"
#include "augs/math/declare_math.h"

struct particle_effect_modifier {
	// GEN INTROSPECTOR struct particle_effect_modifier
	rgba color = white;
	real32 scale_amounts = 1.f;
	real32 scale_lifetimes = 1.f;
	// END GEN INTROSPECTOR

	auto& operator*=(const real32 scalar) {
		scale_amounts *= scalar;
		scale_lifetimes *= scalar;
		return *this;
	}

	bool operator==(const particle_effect_modifier&) const = default;
};	
