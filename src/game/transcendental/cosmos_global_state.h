#pragma once
#include "augs/math/si_scaling.h"

#include "game/global_state/visibility_settings.h"
#include "game/global_state/pathfinding_settings.h"
#include "game/global_state/global_assets.h"
#include "game/global_state/entity_names_meta.h"

struct cosmos_global_state {
	// GEN INTROSPECTOR struct cosmos_global_state
	visibility_settings visibility;
	pathfinding_settings pathfinding;
	si_scaling si;

	entity_names_meta names_meta;
	global_assets assets;
	// END GEN INTROSPECTOR
};