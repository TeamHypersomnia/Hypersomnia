#pragma once
#include "game/modes/ai/behaviors/ai_behavior_push.hpp"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/modes/ai/tasks/ai_waypoint_helpers.hpp"

/*
	Implementation of ai_behavior_push::process().
	Separated into a .hpp file for proper template instantiation.
*/

template <typename T>
std::optional<ai_behavior_patrol> ai_behavior_push::process(
	T& cosm,
	arena_mode_ai_state& ai_state,
	arena_mode_ai_team_state& team_state,
	const mode_player_id& bot_player_id,
	randomization& rng,
	const bool pathfinding_just_completed
) {
	if (!pathfinding_just_completed) {
		return std::nullopt;
	}

	AI_LOG("Reached push waypoint - transitioning to patrol");
	const auto wp_handle = cosm[target_waypoint];

	/*
		Set up camp if the waypoint is a camp waypoint.
	*/
	ai_behavior_patrol new_patrol;
	new_patrol.going_to_first_waypoint = true;

	if (wp_handle.alive() && ::is_camp_waypoint(cosm, target_waypoint)) {
		const auto wp_transform = wp_handle.get_logic_transform();
		const auto [min_secs, max_secs] = ::get_waypoint_camp_duration_range(cosm, target_waypoint);
		new_patrol.camp_timer = rng.randval(min_secs, max_secs);
		new_patrol.camp_duration = new_patrol.camp_timer;
		new_patrol.camp_center = wp_transform.pos;
		new_patrol.camp_twitch_target = wp_transform.pos;
		new_patrol.camp_look_direction = wp_transform.get_direction();
	}

	::unassign_bot_from_waypoints(team_state, bot_player_id);
	ai_state.has_pushed_already = true;

	return new_patrol;
}
