#pragma once
#include "augs/math/vec2.h"
#include "game/cosmos/entity_id.h"
#include "game/enums/marker_type.h"

/*
	Patrol behavior.
	Bot is patrolling between waypoints in an assigned area.
	
	Patrol state includes:
	- Whether we're going to the first waypoint (can sprint/holster)
	- Current waypoint we're heading to
	- Camp state if the current waypoint is a camp waypoint
*/

struct ai_behavior_patrol {
	// GEN INTROSPECTOR struct ai_behavior_patrol
	entity_id current_waypoint;
	bool going_to_first_waypoint = true;
	bool walk_silently_to_next_waypoint = true;

	/*
		Camping state - only active if camp_timer > 0.
	*/
	float camp_timer = 0.0f;
	float camp_duration = 0.0f;
	vec2 camp_center = vec2::zero;
	vec2 camp_twitch_target = vec2::zero;
	vec2 camp_look_direction = vec2(1.0f, 0.0f);

	/*
		Twitch timers for camp movement.
	*/
	float twitch_move_timer = 0.0f;
	float twitch_still_timer = 0.0f;
	float twitch_move_duration = 0.0f;
	float twitch_still_duration = 0.0f;
	bool twitch_is_moving = false;
	// END GEN INTROSPECTOR

	bool is_camping() const {
		return camp_timer > 0.0f && !going_to_first_waypoint;
	}

	bool operator==(const ai_behavior_patrol&) const = default;
};
