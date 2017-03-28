#pragma once
#include "game/transcendental/entity_id.h"

typedef int resource_id_type;

struct play_scene {
	// GEN INTROSPECTOR struct play_scene
	resource_id_type id = 0;
	double at_time = 0.0;
	// END GEN INTROSPECTOR
};

struct play_sound {
	// GEN INTROSPECTOR struct play_sound
	resource_id_type id = 0;
	double at_time = 0.0;
	// END GEN INTROSPECTOR
};

struct focus_guid {
	// GEN INTROSPECTOR struct focus_guid
	entity_guid guid;
	double at_time = 0.0;
	// END GEN INTROSPECTOR
};

struct focus_index {
	// GEN INTROSPECTOR struct focus_index
	unsigned index = 0u;
	double at_time = 0.0;
	// END GEN INTROSPECTOR
};

struct set_sfx_gain {
	// GEN INTROSPECTOR struct set_sfx_gain
	float gain = 0.f;
	double at_time = 0.0;
	// END GEN INTROSPECTOR
};

struct speed_change {
	// GEN INTROSPECTOR struct speed_change
	double speed_multiplier = 1.0;

	double at_time = 0.0;
	double to_time = 0.0;
	// END GEN INTROSPECTOR
};