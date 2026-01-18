#pragma once
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "augs/misc/randomization.h"

/*
	Update camp twitching behavior with breaks.
	
	Camp twitching works in phases:
	1) Twitch phase (100-300ms): Bot moves towards a random direction within 40px radius
	2) Still phase (500-800ms): Bot stays still
	
	If the bot exceeds the twitch radius boundary, it always moves back to center
	regardless of the current phase.
	
	At the start of each twitch phase, a new random target direction is chosen.
*/

inline vec2 update_camp_twitch(
	arena_mode_ai_state& ai_state,
	const vec2 bot_pos,
	const real32 dt,
	randomization& rng
) {
	constexpr float TWITCH_RADIUS = 40.0f;
	
	/* Twitch move duration: 100-300ms */
	constexpr float TWITCH_MOVE_MIN_SECS = 0.1f;
	constexpr float TWITCH_MOVE_MAX_SECS = 0.3f;
	
	/* Still (break) duration: 500-800ms */
	constexpr float TWITCH_STILL_MIN_SECS = 0.5f;
	constexpr float TWITCH_STILL_MAX_SECS = 0.8f;

	const auto offset_from_center = bot_pos - ai_state.camp_center;
	const auto dist_from_center = offset_from_center.length();

	/*
		If we've exceeded the twitch radius, always go back to center.
		This takes priority over the twitch/still phase.
	*/
	if (dist_from_center > TWITCH_RADIUS) {
		const auto direction = ai_state.camp_center - bot_pos;
		if (direction.is_nonzero()) {
			return vec2(direction).normalize();
		}
		return vec2::zero;
	}

	/*
		Update twitch timers based on current phase.
	*/
	if (ai_state.twitch_is_moving) {
		/* In twitch move phase. */
		ai_state.twitch_move_timer -= dt;
		
		if (ai_state.twitch_move_timer <= 0.0f) {
			/* Twitch move phase ended - start still phase. */
			ai_state.twitch_is_moving = false;
			ai_state.twitch_still_duration = rng.randval(TWITCH_STILL_MIN_SECS, TWITCH_STILL_MAX_SECS);
			ai_state.twitch_still_timer = ai_state.twitch_still_duration;
			return vec2::zero;
		}
		
		/* Continue moving towards twitch target. */
		const auto direction = ai_state.camp_twitch_target - bot_pos;
		if (direction.is_nonzero()) {
			return vec2(direction).normalize();
		}
		return vec2::zero;
	}
	else {
		/* In still (break) phase. */
		ai_state.twitch_still_timer -= dt;
		
		if (ai_state.twitch_still_timer <= 0.0f) {
			/* Still phase ended - start new twitch move phase. */
			ai_state.twitch_is_moving = true;
			ai_state.twitch_move_duration = rng.randval(TWITCH_MOVE_MIN_SECS, TWITCH_MOVE_MAX_SECS);
			ai_state.twitch_move_timer = ai_state.twitch_move_duration;
			
			/* Choose a new random twitch target direction. */
			const auto random_angle = rng.randval(0.0f, 360.0f);
			const auto random_offset = vec2::from_degrees(random_angle) * TWITCH_RADIUS * 0.8f;
			ai_state.camp_twitch_target = ai_state.camp_center + random_offset;
			
			/* Move towards the new target. */
			const auto direction = ai_state.camp_twitch_target - bot_pos;
			if (direction.is_nonzero()) {
				return vec2(direction).normalize();
			}
		}
		
		/* Still in break phase - don't move. */
		return vec2::zero;
	}
}
