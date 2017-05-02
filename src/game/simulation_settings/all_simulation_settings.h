#pragma once
#include "augs/math/si_scaling.h"

#include "game/simulation_settings/visibility_settings.h"
#include "game/simulation_settings/pathfinding_settings.h"

struct all_simulation_settings {
	// GEN INTROSPECTOR struct all_simulation_settings
	visibility_settings visibility;
	pathfinding_settings pathfinding;
	si_scaling si;
	// END GEN INTROSPECTOR
};