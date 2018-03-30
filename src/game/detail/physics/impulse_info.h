#pragma once

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
