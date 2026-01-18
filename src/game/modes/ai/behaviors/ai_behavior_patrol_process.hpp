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

inline void ai_behavior_patrol::process(ai_behavior_process_ctx& ctx) {
	auto& cosm = ctx.cosm;
	auto& ai_state = ctx.ai_state;
	const auto& character_pos = ctx.character_pos;
	const auto dt_secs = ctx.dt_secs;
	auto& rng = ctx.rng;
	const bool pathfinding_just_completed = ctx.pathfinding_just_completed;

	/*
		Handle push waypoint first (has priority).
	*/
	if (is_pushing()) {
		if (pathfinding_just_completed) {
			AI_LOG("Reached push waypoint - clearing and switching to normal patrol");
			const auto wp_handle = cosm[push_waypoint];

			/*
				Set up camp if the push waypoint is a camp waypoint.
			*/
			if (wp_handle.alive() && ::is_camp_waypoint(cosm, push_waypoint)) {
				const auto wp_transform = wp_handle.get_logic_transform();
				const auto [min_secs, max_secs] = ::get_waypoint_camp_duration_range(cosm, push_waypoint);
				camp_timer = rng.randval(min_secs, max_secs);
				camp_duration = camp_timer;
				camp_center = wp_transform.pos;
				camp_twitch_target = wp_transform.pos;
				camp_look_direction = wp_transform.get_direction();
			}

			push_waypoint = entity_id::dead();
			ai_state.tried_push_already = true;
			going_to_first_waypoint = true;
		}
		/* Still pushing - don't process normal patrol logic. */
		return;
	}

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
	if (current_waypoint.is_set() && camp_duration > 0.0f && camp_timer <= 0.0f) {
		AI_LOG("Camp duration expired - clearing waypoint for next pick");
		current_waypoint = entity_id::dead();
		camp_duration = 0.0f;
	}

	/*
		Handle pathfinding completion for normal patrol.
	*/
	if (pathfinding_just_completed && current_waypoint.is_set()) {
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
				AI_LOG("Non-camp waypoint - clearing for next pick");
				current_waypoint = entity_id::dead();
			}
		}
	}
}
