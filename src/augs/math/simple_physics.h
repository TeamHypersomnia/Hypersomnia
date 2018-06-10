#pragma once
#include "augs/math/physics_structs.h"

struct simple_rot_vel {
	// GEN INTROSPECTOR struct simple_rot_vel
	real32 rotation = 0.f;
	real32 angular_velocity = 0.f;
	// END GEN INTROSPECTOR

	void apply(const real32 in) {
		angular_velocity += in;
	}

	void integrate(const real32 dt) {
		rotation += angular_velocity * dt;
	}

	void damp(const real32 dt, const real32 angular) {
		angular_velocity = augs::damp(angular_velocity, dt, angular);
	}
};

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
