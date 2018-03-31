#pragma once
#include "augs/math/declare_math.h"

struct damping_mults {
	// GEN INTROSPECTOR struct damping_mults
	real32 linear = 0.f;
	vec2 linear_axis_aligned;
	real32 angular = 0.f;
	// END GEN INTROSPECTOR
};

struct impulse_mults {
	// GEN INTROSPECTOR struct impulse_mults
	real32 linear = 0.f;
	real32 angular = 0.f;
	// END GEN INTROSPECTOR

	auto operator+(const impulse_mults& b) const {
		return impulse_mults { linear + b.linear, angular + b.angular };
	}
};

struct impulse_input {
	vec2 linear;
	real32 angular = 0.f;

	auto operator*(const impulse_mults& b) const {
		return impulse_input { linear * b.linear, angular * b.angular };
	}
};

struct damping_input {
	// GEN INTROSPECTOR struct damping_input
	vec2 linear;
	real32 angular = 0.f;
	// END GEN INTROSPECTOR
};
