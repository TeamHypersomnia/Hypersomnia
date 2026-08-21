#pragma once
#include "augs/math/vec2.h"

/*
	These are ruleset values, derived from the map JSON - not player-editable.
	The size doubles as the snap-to-fov target for the camera zoom, so the
	balance zoom-out is folded into the fov expansion (see
	get_camera_requested_fov_expansion) rather than baked into this size.
*/

struct fog_of_war_settings {
	// GEN INTROSPECTOR struct fog_of_war_settings
	real32 angle = 165.f;
	vec2 size = vec2(1920, 1080);
	// END GEN INTROSPECTOR

	bool operator==(const fog_of_war_settings& b) const = default;

	bool is_enabled() const {
		return angle > 0.f && angle < 360.f;
	}

	auto get_real_size() const {
		return size * 2;
	}
};
