#pragma once
#include "game/modes/ai/behaviors/ai_behavior_patrol.hpp"
#include "game/modes/ai/behaviors/ai_behavior_process_ctx.hpp"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/modes/ai/tasks/ai_waypoint_helpers.hpp"

/*
	Update camp twitching behavior with breaks.
	
	Camp twitching works in phases:
	1) Twitch phase (100-300ms): Bot moves towards a random direction within 40px radius
	2) Still phase (500-800ms): Bot stays still
	
	If the bot exceeds the twitch radius boundary, it always moves back to center
	regardless of the current phase.
	
	At the start of each twitch phase, a new random target direction is chosen.
*/

inline void update_camp_twitch(
	ai_behavior_patrol& patrol,
	const vec2 bot_pos,
	const real32 dt,
	randomization& rng
) {
	constexpr float TWITCH_RADIUS = 40.0f;
	
	/* Twitch move duration: 100-300ms */
	constexpr float TWITCH_MOVE_MIN_SECS = 0.1f;
	constexpr float TWITCH_MOVE_MAX_SECS = 0.5f;
	
	/* Still (break) duration: 500-800ms */
	constexpr float TWITCH_STILL_MIN_SECS = 0.4f;
	constexpr float TWITCH_STILL_MAX_SECS = 3.0f;

	const auto offset_from_center = bot_pos - patrol.camp_center;
	const auto dist_from_center = offset_from_center.length();

	/*
		If we've exceeded the twitch radius, always go back to center.
		This takes priority over the twitch/still phase.
	*/
	if (dist_from_center > TWITCH_RADIUS) {
		patrol.camp_twitch_target = (patrol.camp_center - bot_pos).normalize();
	}

	auto set_twitch_direction = [&](const vec2& dir) {
		if (dir.is_nonzero()) {
			patrol.twitch_direction = vec2(dir).normalize();
		}
		else {
			patrol.twitch_direction = std::nullopt;
		}
	};

	/*
		Update twitch timers based on current phase.
	*/
	if (patrol.twitch_is_moving) {
		/* In twitch move phase. */
		patrol.twitch_move_timer -= dt;
		
		if (patrol.twitch_move_timer <= 0.0f) {
			/* Twitch move phase ended - start still phase. */
			patrol.twitch_is_moving = false;
			patrol.twitch_still_duration = rng.randval(TWITCH_STILL_MIN_SECS, TWITCH_STILL_MAX_SECS);
			patrol.twitch_still_timer = patrol.twitch_still_duration;
			patrol.twitch_direction = std::nullopt;
			return;
		}
		
		/* Continue moving towards twitch target. */
		set_twitch_direction(patrol.camp_twitch_target);
	}
	else {
		/* In still (break) phase. */
		patrol.twitch_still_timer -= dt;
		
		if (patrol.twitch_still_timer <= 0.0f) {
			/* Still phase ended - start new twitch move phase. */
			patrol.twitch_is_moving = true;
			patrol.twitch_move_duration = rng.randval(TWITCH_MOVE_MIN_SECS, TWITCH_MOVE_MAX_SECS);
			patrol.twitch_move_timer = patrol.twitch_move_duration;
			
			/* Choose a new random twitch target direction. */
			const auto random_angle = rng.randval(0.0f, 360.0f);
			patrol.camp_twitch_target = vec2::from_degrees(random_angle);

			if (dist_from_center > TWITCH_RADIUS) {
				patrol.camp_twitch_target = (patrol.camp_center - bot_pos).normalize();
			}
			
			set_twitch_direction(patrol.camp_twitch_target);
		}
		else {
			/* Still in break phase - don't move. */
			patrol.twitch_direction = std::nullopt;
		}
	}
}

/*
	Implementation of ai_behavior_patrol::process().
	
	Handles:
	- Push waypoint logic (priority over normal patrol)
	- Camp timer countdown
	- Camp twitch updates
	- Waypoint picking when path completes
*/

inline assigned_waypoint_result ai_behavior_patrol::calc_assigned_waypoint() const {
	assigned_waypoint_result result;
	/*
		Push waypoint takes priority over patrol waypoint.
	*/

	if (push_waypoint.is_set()) {
		result.waypoint_id = push_waypoint;
		result.is_push_waypoint = true;
	}
	else if (patrol_waypoint.is_set()) {
		result.waypoint_id = patrol_waypoint;
		result.is_push_waypoint = false;
	}

	return result;
}

inline void ai_behavior_patrol::clear_waypoint() {
	patrol_waypoint = entity_id::dead();
	push_waypoint = entity_id::dead();
	camp_duration = 0.0f;
}

inline bool ai_behavior_patrol::is_going_far() const {
	if (is_camping()) {
		return false;
	}

	if (is_pushing()) {
		return true;
	}

	return going_to_first_waypoint;
}

inline void ai_behavior_patrol::process(ai_behavior_process_ctx& ctx) {
	auto& cosm = ctx.cosm;
	auto& ai_state = ctx.ai_state;
	auto& team_state = ctx.team_state;
	const auto& bot_player_id = ctx.bot_player_id;
	const auto& character_pos = ctx.character_pos;
	const auto dt_secs = ctx.dt_secs;
	auto& rng = ctx.rng;
	const bool pathfinding_just_completed = ctx.pathfinding_just_completed;

	auto now_waypoint = calc_assigned_waypoint();
	const auto current_waypoint_id = now_waypoint.waypoint_id;
	const auto current_is_push = now_waypoint.is_push_waypoint;

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
		::update_camp_twitch(*this, character_pos, dt_secs, rng);
	}

	/*
		Check if camp timer just expired - need to pick next waypoint.
	*/
	if (current_waypoint_id.is_set() && camp_duration > 0.0f && camp_timer <= 0.0f) {
		AI_LOG("Camp duration expired - clearing waypoint for next pick");
		clear_waypoint();

		/*
			PHASE 4 (calc_pathfinding_request) ran before this process() call
			and may have started pathfinding back to the old camp waypoint
			(since is_camping() was already false but patrol_waypoint was still set).

			Clear that spurious pathfinding so it doesn't immediately complete
			next frame (bot is already at old position) and falsely trigger
			pathfinding_just_completed on whatever new waypoint we pick below.
		*/
		ai_state.clear_pathfinding();
		ai_state.current_pathfinding_request = std::nullopt;
	}

	/*
		Handle pathfinding completion.
	*/
	if (pathfinding_just_completed) {
		AI_LOG("Reached patrol waypoint");
		const auto wp_handle = cosm[current_waypoint_id];

		if (wp_handle.alive()) {
			if (::is_camp_waypoint(cosm, current_waypoint_id)) {
				AI_LOG("Camp waypoint - setting up camp");
				const auto wp_transform = wp_handle.get_logic_transform();
				const auto [min_secs, max_secs] = ::get_waypoint_camp_duration_range(cosm, current_waypoint_id);

				camp_timer = rng.randval(min_secs, max_secs);

				if (!walk_silently_to_next_waypoint) {
					/*
						If we were loud coming here,
						stay only a moment.
					*/
					camp_timer /= 5.0f;
					camp_timer = std::min(camp_timer, 2.0f);
				}

				camp_duration = camp_timer;
				camp_center = wp_transform.pos;
				camp_twitch_target = wp_transform.pos;
				camp_look_direction = wp_transform.get_direction();
			}
			else {
				AI_LOG("Non-camp waypoint - clearing for next pick");
				clear_waypoint();
			}
		}
	}

	/*
		Find a new patrol waypoint if current one is dead.
		This advances the patrol sequence.
	*/
	if (!calc_assigned_waypoint().waypoint_id.is_set()) {
		/*
			Might have been a push waypoint:
		   	sprint back to the actual patrol waypoint.
		*/

		const auto new_wp = ::find_random_unassigned_patrol_waypoint(
			cosm,
			team_state,
			ai_state.patrol_letter,
			bot_player_id,
			current_waypoint_id,
			rng
		);

		if (new_wp.is_set()) {
			going_to_first_waypoint = current_is_push || !current_waypoint_id.is_set();
			walk_silently_to_next_waypoint = rng.randval(0, 99) > 18;

			AI_LOG("Found new patrol waypoint");
			patrol_waypoint = new_wp;
		}
	}
}
