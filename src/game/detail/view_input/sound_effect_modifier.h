#pragma once
#include "augs/math/declare_math.h"
#include "augs/audio/distance_model.h"

struct sound_effect_modifier {
	// GEN INTROSPECTOR struct sound_effect_modifier
	real32 gain = 1.f;
	real32 pitch = 1.f;
	real32 max_distance = -1.f;
	real32 reference_distance = -1.f;
	real32 doppler_factor = 1.f;
	char repetitions = 1;
	bool fade_on_exit = true;
	bool disable_velocity = false;
	bool always_direct_listener = false;
	augs::distance_model distance_model = augs::distance_model::NONE;
	// END GEN INTROSPECTOR
};
