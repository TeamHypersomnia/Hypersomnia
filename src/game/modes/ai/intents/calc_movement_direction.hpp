#pragma once
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/detail/pathfinding/navigate_pathfinding.hpp"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/modes/ai/behaviors/ai_behavior_variant.hpp"
#include "game/modes/ai/intents/should_helpers.hpp"

/*
	Result of movement direction calculation.
*/

struct movement_direction_result {
	std::optional<vec2> direction;
	vec2 crosshair_offset = vec2::zero;
	bool path_completed = false;
	bool is_navigating = false;
	bool can_sprint = false;  /* True when movement direction is mostly parallel to path direction (within ~15 degrees). */
};

/*
	Update camp twitching behavior with breaks.
	
	Camp twitching works in phases:
	1) Twitch phase (100-300ms): Bot moves towards a random direction within 40px radius
	2) Still phase (500-800ms): Bot stays still
	
	If the bot exceeds the twitch radius boundary, it always moves back to center
	regardless of the current phase.
	
	At the start of each twitch phase, a new random target direction is chosen.
*/

inline std::optional<vec2> update_camp_twitch(
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
			return std::nullopt;
		}
		
		/* Continue moving towards twitch target. */
		const auto direction = patrol.camp_twitch_target;

		if (direction.is_nonzero()) {
			return vec2(direction).normalize();
		}
		return std::nullopt;
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
			const auto random_offset = vec2::from_degrees(random_angle);
			patrol.camp_twitch_target = random_offset;

			if (dist_from_center > TWITCH_RADIUS) {
				patrol.camp_twitch_target = (patrol.camp_center - bot_pos).normalize();
			}
			
			/* Move towards the new target. camp_twitch_target is already a direction. */
			const auto direction = patrol.camp_twitch_target;
			if (direction.is_nonzero()) {
				return vec2(direction).normalize();
			}
		}
		
		/* Still in break phase - don't move. */
		return std::nullopt;
	}
}

/*
	Stateless calculation of the current movement direction.
	
	This function determines HOW the bot should move based on:
	- Current behavior type
	- Current pathfinding state (if navigating to a target)
	- Camp twitching (if camping on a waypoint)
	- Defusing (no movement)
	
	Also outputs crosshair offset for aiming.
	Uses std::visit on the behavior variant.
*/

template <typename CharacterHandle>
inline movement_direction_result calc_current_movement_direction(
	ai_behavior_variant& behavior,
	std::optional<ai_pathfinding_state>& pathfinding,
	const vec2 character_pos,
	const cosmos_navmesh& navmesh,
	CharacterHandle character,
	const real32 dt,
	randomization& rng
) {
	movement_direction_result result;

	/*
		Check if defusing - don't move at all.
	*/
	if (const auto* defuse = ::get_behavior_if<ai_behavior_defuse>(behavior)) {
		if (defuse->is_defusing) {
			return result;
		}
	}

	/*
		Check if camping (patrol with camp timer > 0) - do camp twitching.
	*/
	if (auto* patrol = ::get_behavior_if<ai_behavior_patrol>(behavior)) {
		if (patrol->is_camping()) {
			const auto twitch_dir = ::update_camp_twitch(*patrol, character_pos, dt, rng);
			result.direction = twitch_dir;
			/*
				Look in the direction of the waypoint transform while camping.
			*/
			result.crosshair_offset = patrol->camp_look_direction * 200.0f;
			return result;
		}
	}

	/*
		If pathfinding is active, use navigate_pathfinding for proper path following.
	*/
	if (pathfinding.has_value()) {
		const auto nav_result = ::navigate_pathfinding(
			pathfinding,
			character_pos,
			navmesh,
			character,
			dt
		);

		result.path_completed = nav_result.path_completed;
		result.is_navigating = nav_result.is_navigating;
		result.crosshair_offset = nav_result.crosshair_offset;
		result.can_sprint = nav_result.can_sprint;

		if (nav_result.is_navigating) {
			result.direction = nav_result.movement_direction;
		}
		else if (nav_result.path_completed) {
			/*
				Path completed - don't move, but set crosshair to face target direction.
			*/
			result.crosshair_offset = nav_result.crosshair_offset;
		}
	}

	return result;
}
