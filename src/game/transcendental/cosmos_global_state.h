#pragma once
#include "augs/math/si_scaling.h"

#include "game/global_state/visibility_settings.h"
#include "game/global_state/pathfinding_settings.h"
#include "game/global_state/global_assets.h"
#include "game/global_state/entity_name_metas.h"

#include "game/detail/spells/all_spells.h"

struct cosmos_global_state {
	// GEN INTROSPECTOR struct cosmos_global_state
	visibility_settings visibility;
	pathfinding_settings pathfinding;
	si_scaling si;

	entity_name_metas name_metas;
	global_assets assets;

	put_all_meters_into_t<std::tuple> meters;
	put_all_spells_into_t<std::tuple> spells;
	put_all_perks_into_t<std::tuple> perks;
	// END GEN INTROSPECTOR
};