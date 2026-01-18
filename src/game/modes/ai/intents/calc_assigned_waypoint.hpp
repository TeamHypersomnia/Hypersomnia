#pragma once
#include "game/cosmos/entity_id.h"
#include "game/modes/ai/behaviors/ai_behavior_variant.hpp"


/*
	Stateless calculation of the currently assigned waypoint for a bot.
*/

inline assigned_waypoint_result calc_assigned_waypoint(
	const ai_behavior_variant& behavior
) {
	if (const auto* patrol = ::get_behavior_if<ai_behavior_patrol>(behavior)) {
		return patrol->calc_assigned_waypoint();
	}

	return assigned_waypoint_result();
}
