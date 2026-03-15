#pragma once
#include "game/components/crosshair_component.h"
#include "game/modes/ai/ai_character_context.h"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/modes/difficulty_type.h"

/*
	Linear crosshair movement with acceleration profile.
	Moves faster when far away, decelerates near the target for accurate aim.
*/

/* Speed curve parameters (pixels per second). */
static constexpr float CROSSHAIR_BASE_SPEED = 1000.0f;
static constexpr float CROSSHAIR_MAX_SPEED  = 2000.0f;

/*
	Distance thresholds for the speed curve.
	Far (>= far_dist): base_speed
	Mid (~mid_dist):    max_speed (peak acceleration)
	Near (approaching 0): base_speed (deceleration for precise aim)
*/
static constexpr float CROSSHAIR_SPEED_FAR_DIST = 1000.0f;
static constexpr float CROSSHAIR_SPEED_MID_DIST = 500.0f;

/* Difficulty multipliers for crosshair movement speed. */
inline float get_crosshair_speed_mult(const difficulty_type difficulty) {
	switch (difficulty) {
		case difficulty_type::EASY:   return 0.4f;
		case difficulty_type::MEDIUM: return 0.7f;
		case difficulty_type::HARD:   return 1.2f;
		default:                      return 1.0f;
	}
}

/*
	Compute the movement speed based on distance to target.
	Profile: base_speed at far -> ramps up to max_speed at mid -> back to base_speed near target.
*/
inline float crosshair_speed_for_distance(const float dist) {
	if (dist >= CROSSHAIR_SPEED_FAR_DIST) {
		return CROSSHAIR_BASE_SPEED;
	}

	if (dist >= CROSSHAIR_SPEED_MID_DIST) {
		/* Ramp up from base to max as we go from far to mid. */
		const float t = (CROSSHAIR_SPEED_FAR_DIST - dist) / (CROSSHAIR_SPEED_FAR_DIST - CROSSHAIR_SPEED_MID_DIST);
		return CROSSHAIR_BASE_SPEED + (CROSSHAIR_MAX_SPEED - CROSSHAIR_BASE_SPEED) * t;
	}

	/* Ramp down from max to base as we approach 0. */
	const float t = dist / CROSSHAIR_SPEED_MID_DIST;
	return CROSSHAIR_BASE_SPEED + (CROSSHAIR_MAX_SPEED - CROSSHAIR_BASE_SPEED) * t;
}

template <typename CrosshairHandle>
inline void interpolate_crosshair(
	CrosshairHandle crosshair,
	const vec2 target_offset,
	const float dt_secs,
	const difficulty_type difficulty,
	const bool is_navigating = false
) {
	(void)is_navigating;
	if (crosshair) {
		{
			const auto current = crosshair->base_offset;
			const auto diff = target_offset - current;
			const auto dist = diff.length();

			const auto speed = crosshair_speed_for_distance(dist) * get_crosshair_speed_mult(difficulty);
			const auto step = speed * dt_secs;

			if (step >= dist) {
				crosshair->base_offset = target_offset;
			}
			else {
				crosshair->base_offset = current + diff * (step / dist);
			}
		}
	}
}

/*
	Overload that takes ai_character_context for convenience.
*/

inline void interpolate_crosshair(
	const ai_character_context& ctx,
	const vec2 target_crosshair_offset,
	const float dt_secs,
	const difficulty_type difficulty,
	const bool is_navigating = false
) {
	::interpolate_crosshair(
		ctx.character_handle.find_crosshair(),
		target_crosshair_offset,
		dt_secs,
		difficulty,
		is_navigating
	);
}
