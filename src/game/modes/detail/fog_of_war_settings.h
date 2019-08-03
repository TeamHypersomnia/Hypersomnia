#pragma once
#include "augs/math/vec2.h"

struct fog_of_war_settings {
	// GEN INTROSPECTOR struct fog_of_war_settings
	real32 angle = 180.f;
	vec2 size = vec2(1920, 1080);
	// END GEN INTROSPECTOR

	bool is_enabled() const {
		return angle > 0.f && angle < 360.f;
	}

	auto get_real_size() const {
		return size * 2;
	}
};
