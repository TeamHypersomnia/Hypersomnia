#pragma once
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "augs/misc/randomization.h"

/*
	Update camp twitching behavior.
	Bot moves randomly within a radius of 40px of the camp center
	but keeps looking in the same direction.
*/

inline vec2 update_camp_twitch(
	arena_mode_ai_state& ai_state,
	const vec2 bot_pos,
	randomization& rng
) {
	constexpr float TWITCH_RADIUS = 40.0f;
	constexpr float CENTER_EPSILON = 10.0f;

	const auto offset_from_center = bot_pos - ai_state.camp_center;
	const auto dist_from_center = offset_from_center.length();

	/*
		If we've exceeded the twitch radius, go back to center.
	*/
	if (dist_from_center > TWITCH_RADIUS) {
		ai_state.camp_twitch_target = ai_state.camp_center;
	}
	/*
		If we're close to the center, choose a new random direction.
	*/
	else if ((bot_pos - ai_state.camp_twitch_target).length() < CENTER_EPSILON) {
		const auto random_angle = rng.randval(0.0f, 360.0f);
		const auto random_offset = vec2::from_degrees(random_angle) * TWITCH_RADIUS * 0.8f;
		ai_state.camp_twitch_target = ai_state.camp_center + random_offset;
	}

	return (ai_state.camp_twitch_target - bot_pos).normalize();
}
