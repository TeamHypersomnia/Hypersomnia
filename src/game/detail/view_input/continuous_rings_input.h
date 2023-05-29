#pragma once

/*
	Note the radius is implied by other properties like object size.
*/

struct continuous_rings_input {
	// GEN INTROSPECTOR struct continuous_rings_input
	float effect_speed = 1.0f;

	rgba inner_color = cyan;
	rgba outer_color = turquoise;
	// END GEN INTROSPECTOR

	bool operator==(const continuous_rings_input& b) const = default;
};
