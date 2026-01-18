#pragma once
#include "game/cosmos/entity_id.h"
#include "game/modes/ai/behaviors/ai_behavior_variant.hpp"

/*
	Result of waypoint assignment calculation.
*/

struct assigned_waypoint_result {
	entity_id waypoint_id;
	bool is_push_waypoint = false;
};

/*
	Stateless calculation of the currently assigned waypoint for a bot.
	
	This function determines which waypoint the bot should be assigned to based on:
	- Current behavior type (patrol with push_waypoint takes priority)
	- Current patrol waypoint
	
	This is used to update the waypoint assignment caches statelessly
	before calling update_arena_mode_ai.
*/

inline assigned_waypoint_result calc_assigned_waypoint(
	const ai_behavior_variant& behavior
) {
	assigned_waypoint_result result;

	if (const auto* patrol = ::get_behavior_if<ai_behavior_patrol>(behavior)) {
		/*
			Push waypoint takes priority over patrol waypoint.
		*/
		if (patrol->push_waypoint.is_set()) {
			result.waypoint_id = patrol->push_waypoint;
			result.is_push_waypoint = true;
		}
		else if (patrol->current_waypoint.is_set()) {
			result.waypoint_id = patrol->current_waypoint;
			result.is_push_waypoint = false;
		}
	}

	return result;
}
