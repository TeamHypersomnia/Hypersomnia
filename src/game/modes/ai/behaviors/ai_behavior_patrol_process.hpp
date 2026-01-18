#pragma once
#include "game/modes/ai/behaviors/ai_behavior_patrol.hpp"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/modes/ai/tasks/ai_waypoint_helpers.hpp"

/*
	Implementation of ai_behavior_patrol::process().
	Separated into a .hpp file for proper template instantiation.
*/

template <typename T>
void ai_behavior_patrol::process(
	T& cosm,
	arena_mode_ai_state& ai_state,
	arena_mode_ai_team_state& team_state,
	const mode_player_id& bot_player_id,
	const vec2 character_pos,
	const real32 dt_secs,
	randomization& rng,
	const bool pathfinding_just_completed
) {
	/*
		Update camp timer.
	*/
	if (camp_timer > 0.0f) {
		camp_timer -= dt_secs;
	}

	/*
		If camping, update twitch movement.
	*/
	if (is_camping()) {
		update_camp_twitch(character_pos, dt_secs, rng);
	}

	/*
		Check if camp timer just expired - need to pick next waypoint.
	*/
	if (current_waypoint.is_set() && camp_duration > 0.0f && camp_timer <= 0.0f) {
		AI_LOG("Camp duration expired - picking next waypoint");
		::unassign_bot_from_waypoints(team_state, bot_player_id);
		current_waypoint = entity_id::dead();
		camp_duration = 0.0f;
	}

	/*
		Handle pathfinding completion.
	*/
	if (pathfinding_just_completed) {
		AI_LOG("Reached patrol waypoint");
		going_to_first_waypoint = false;

		const auto wp_handle = cosm[current_waypoint];

		if (wp_handle.alive()) {
			if (::is_camp_waypoint(cosm, current_waypoint)) {
				AI_LOG("Camp waypoint - setting up camp");
				const auto wp_transform = wp_handle.get_logic_transform();
				const auto [min_secs, max_secs] = ::get_waypoint_camp_duration_range(cosm, current_waypoint);
				camp_timer = rng.randval(min_secs, max_secs);
				camp_duration = camp_timer;
				camp_center = wp_transform.pos;
				camp_twitch_target = wp_transform.pos;
				camp_look_direction = wp_transform.get_direction();
			}
			else {
				AI_LOG("Non-camp waypoint - picking next");
				::unassign_bot_from_waypoints(team_state, bot_player_id);
				current_waypoint = entity_id::dead();
			}
		}
	}

	/*
		Find a waypoint if we don't have one.
	*/
	if (!current_waypoint.is_set()) {
		AI_LOG("No waypoint - finding one");
		const auto new_wp = ::find_random_unassigned_patrol_waypoint(
			cosm,
			team_state,
			ai_state.patrol_letter,
			bot_player_id,
			entity_id::dead(),
			rng
		);

		if (new_wp.is_set()) {
			current_waypoint = new_wp;
			::assign_waypoint(team_state, new_wp, bot_player_id);
			AI_LOG_NVPS(new_wp);

			/* 85% chance to walk silently when not going to first waypoint. */
			if (!going_to_first_waypoint) {
				walk_silently_to_next_waypoint = rng.randval(0, 99) < 85;
			}
		}
	}
}
