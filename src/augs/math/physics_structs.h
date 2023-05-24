#pragma once
#include "augs/math/vec2.h"

struct damping_mults {
	// GEN INTROSPECTOR struct damping_mults
	real32 linear = 0.f;
	vec2 linear_axis_aligned;
	real32 angular = 0.f;
	// END GEN INTROSPECTOR

	auto& sanitize() {
		auto s = [](real32& f){ f = std::max(0.f, f); };

		s(linear);
		s(angular);
		s(linear_axis_aligned.x);
		s(linear_axis_aligned.y);

		return *this;
	}
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

enum class impulse_type {
	// GEN INTROSPECTOR enum class impulse_type
	FORCE,
	IMPULSE,
	ADD_VELOCITY,
	SET_VELOCITY
	// END GEN INTROSPECTOR
};

struct impulse_amount_def {
	// GEN INTROSPECTOR struct impulse_amount_def
	real32 amount = 1000.0f;
	impulse_type mode = impulse_type::IMPULSE;
	// END GEN INTROSPECTOR

	bool operator==(const impulse_amount_def& b) const = default;
};

struct damping_input {
	// GEN INTROSPECTOR struct damping_input
	vec2 linear;
	real32 angular = 0.f;
	// END GEN INTROSPECTOR
};
