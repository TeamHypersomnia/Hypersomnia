#pragma once
#include "augs/math/si_scaling.h"

#include "game/global_state/visibility_settings.h"
#include "game/global_state/pathfinding_settings.h"
#include "game/global_state/global_assets.h"

struct cosmos_global_state {
	// GEN INTROSPECTOR struct cosmos_global_state
	visibility_settings visibility;
	pathfinding_settings pathfinding;
	si_scaling si;

	global_assets assets;
	// END GEN INTROSPECTOR
};