#pragma once
#include "augs/math/physics_structs.h"

struct simple_body {
	// GEN INTROSPECTOR struct simple_body
	vec2 position;
	real32 rotation = 0.f;

	vec2 linear_velocity;
	real32 angular_velocity = 0.f;
	// END GEN INTROSPECTOR

	void apply(const impulse_input in) {
		linear_velocity += in.linear;
		angular_velocity += in.angular;
	}

	void integrate(const real32 dt) {
		position += linear_velocity * dt;
		rotation += angular_velocity * dt;
	}

	void damp(const real32 dt, const damping_input in) {
		linear_velocity.damp(dt, in.linear);
		angular_velocity = augs::damp(angular_velocity, dt, in.angular);
	}
};
