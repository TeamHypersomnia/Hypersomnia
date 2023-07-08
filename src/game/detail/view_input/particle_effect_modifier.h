#pragma once
#include "augs/graphics/rgba.h"
#include "augs/math/declare_math.h"

struct particle_effect_modifier {
	// GEN INTROSPECTOR struct particle_effect_modifier
	rgba color = white;
	real32 scale_amounts = 1.f;
	real32 scale_lifetimes = 1.f;
	real32 radius = 0.0f;
	vec2 box = vec2::zero;
	// END GEN INTROSPECTOR

	void sanitize() {
		scale_amounts = std::min(scale_amounts, 100.0f);
		scale_lifetimes = std::min(scale_lifetimes, 100.0f);
	}

	auto& operator*=(const real32 scalar) {
		scale_amounts *= scalar;
		scale_lifetimes *= scalar;
		return *this;
	}

	auto& operator*=(const particle_effect_modifier& b) {
		color *= b.color;
		scale_amounts *= b.scale_amounts;
		scale_lifetimes *= b.scale_lifetimes;
		return *this;
	}

	bool operator==(const particle_effect_modifier&) const = default;
};	
