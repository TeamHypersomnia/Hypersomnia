#pragma once
#include "augs/math/vec2.h"

struct arena_playtesting_context {
	// GEN INTROSPECTOR struct arena_playtesting_context
	vec2 spawn_transform;
	// END GEN INTROSPECTOR

	bool operator==(const arena_playtesting_context&) const = default;
};
