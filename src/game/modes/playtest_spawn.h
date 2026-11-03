#pragma once
#include <string>
#include "game/cosmos/entity_id.h"
#include "game/detail/inventory/requested_equipment.h"

struct playtest_spawn {
	// GEN INTROSPECTOR struct playtest_spawn
	entity_id spawn;
	bool custom_equipment = false;
	requested_equipment equipment;
	bool respawns_in_quick_test = false;
	std::string bot_nickname;
	// END GEN INTROSPECTOR
};
