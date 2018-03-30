#pragma once

struct damping_info {
	// GEN INTROSPECTOR struct damping_info
	real32 linear = 0.f;
	vec2 linear_axis_aligned;
	real32 angular = 0.f;
	// END GEN INTROSPECTOR
};

struct impulse_info {
	// GEN INTROSPECTOR struct impulse_info
	float linear = 0.f;
	float angular = 0.f;
	// END GEN INTROSPECTOR

	auto operator+(const impulse_info& b) const {
		return impulse_info { linear + b.linear, angular + b.angular };
	}
};

struct impulse_input {
	vec2 linear;
	float angular = 0.f;
};
