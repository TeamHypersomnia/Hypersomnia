#pragma once
#include "game/cosmos/entity_id.h"

/*
	Push behavior.
	Bot is moving towards a push waypoint before switching to patrol.
	
	Push waypoints are used at round start for aggressive forward positions.
	Once reached, the bot transitions to patrol.
*/

struct ai_behavior_push {
	// GEN INTROSPECTOR struct ai_behavior_push
	entity_id target_waypoint;
	// END GEN INTROSPECTOR

	bool operator==(const ai_behavior_push&) const = default;
};
