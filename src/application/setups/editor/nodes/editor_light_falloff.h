#pragma once
#include "game/components/light_component.h"

/*
	The same three parameters drive both models - see light_falloff.
*/

struct editor_light_falloff {
	// GEN INTROSPECTOR struct editor_light_falloff
	real32 radius = 600.0f;
	real32 intensity = 1.0f;
	real32 falloff = 0.5f;
	// END GEN INTROSPECTOR

	light_falloff to_physical() const {
		return { radius, intensity, falloff };
	}

	real32 calc_attenuation_mult_for_requested_radius() const;

	bool operator==(const editor_light_falloff&) const = default;
};
