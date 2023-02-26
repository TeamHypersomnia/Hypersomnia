#pragma once

struct editor_light_falloff {
	// GEN INTROSPECTOR struct editor_light_falloff
	real32 constant = 0.0f;
	real32 linear = 0.0f;
	real32 quadratic = 1.0f;

	real32 radius = 800.0f;

	rgba_channel cutoff_alpha = 3;
	// END GEN INTROSPECTOR

	real32 calc_attenuation_mult_for_requested_radius() const;
};

