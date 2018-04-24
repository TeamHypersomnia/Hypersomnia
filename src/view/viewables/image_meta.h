#pragma once
#include "augs/math/vec2.h"
#include "augs/drawing/flip.h"

struct image_usage_as_button {
	// GEN INTROSPECTOR struct image_usage_as_button
	flip_flags flip;
	vec2 bbox_expander;
	// END GEN INTROSPECTOR
};

struct image_meta {
	// GEN INTROSPECTOR struct image_meta
	image_usage_as_button usage_as_button;
	// END GEN INTROSPECTOR

	bool operator==(const image_meta& b) const;
};
