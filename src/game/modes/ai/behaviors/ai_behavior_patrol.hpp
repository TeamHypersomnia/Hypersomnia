#pragma once
#include <optional>
#include "augs/math/vec2.h"
#include "game/cosmos/entity_id.h"
#include "game/enums/marker_type.h"

struct arena_mode_ai_team_state;
struct arena_mode_ai_state;
struct mode_player_id;
struct ai_behavior_process_ctx;

/*
	Patrol behavior.
	Bot is patrolling between waypoints in an assigned area.
	
	Patrol state includes:
	- Optional push_waypoint (takes priority if set, for initial push before patrol)
	- Whether we're going to the first waypoint (can sprint/holster)
	- Current waypoint we're heading to
	- Camp state if the current waypoint is a camp waypoint
*/

struct assigned_waypoint_result {
	entity_id waypoint_id;
	bool is_push_waypoint = false;
};

struct ai_behavior_patrol {
	// GEN INTROSPECTOR struct ai_behavior_patrol
	/*
		Optional push waypoint - if set, takes priority over normal patrol.
		Once reached, this is cleared and we fall back to normal patrolling.
	*/
	entity_id push_waypoint;

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

	/*
		Movement result from camp twitching (updated by process()).
	*/
	std::optional<vec2> twitch_direction;
	// END GEN INTROSPECTOR

	bool is_camping() const {
		return camp_timer > 0.0f && !going_to_first_waypoint;
	}

	bool is_pushing() const {
		return push_waypoint.is_set();
	}

	assigned_waypoint_result calc_assigned_waypoint() const;

	bool operator==(const ai_behavior_patrol&) const = default;

	/*
		Process patrol behavior for this frame.
		Handles:
		- Push waypoint logic (priority over normal patrol)
		- Camp timer countdown
		- Camp twitch updates
		- Waypoint picking when path completes
	*/
	void process(ai_behavior_process_ctx& ctx);
};

