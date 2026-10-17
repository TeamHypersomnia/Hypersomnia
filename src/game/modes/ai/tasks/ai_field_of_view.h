#pragma once
#include "augs/math/vec2.h"
#include "augs/math/repro_math.h"
#include "game/balance_params.h"
#include "game/modes/difficulty_type.h"
#include "game/modes/detail/fog_of_war_settings.h"

/*
	The bot's field of view: a box around the bot and a cone around its aim.
*/

struct ai_field_of_view {
	vec2 half_size;
	real32 angle = 360.f;

	bool considers_angle() const {
		return angle > 0.f && angle < 360.f;
	}

	/* The cheap check, done first. */
	bool within_box(const vec2 from_pos, const vec2 target_pos) const {
		const auto offset = target_pos - from_pos;

		return repro::fabs(offset.x) < half_size.x && repro::fabs(offset.y) < half_size.y;
	}

	bool within_angle(const vec2 aim_direction, const vec2 direction_to_target) const {
		if (!considers_angle()) {
			return true;
		}

		return aim_direction.degrees_between(direction_to_target) <= angle / 2;
	}
};

inline float get_bot_fov_mult(const difficulty_type difficulty) {
	switch (difficulty) {
		case difficulty_type::VERY_EASY: return BALANCE_BOT_FOV_MULT_VERY_EASY;
		case difficulty_type::EASY: return BALANCE_BOT_FOV_MULT_EASY;
		case difficulty_type::HARD: return BALANCE_BOT_FOV_MULT_HARD;
		default: return BALANCE_BOT_FOV_MULT_MEDIUM;
	}
}

/*
	A bot sees proportionally to what a player standing in the same spot would:
	the map's fog of war scaled by the camera zoom there (see calc_area_camera_zoom),
	times the difficulty's balance multiplier.
	Looking far is like a player with the crosshair at the screen edge:
	the bot sees farther, but within a much narrower angle.
*/

inline ai_field_of_view calc_ai_field_of_view(
	const fog_of_war_settings& fog_of_war,
	const float camera_zoom,
	const float mult,
	const bool looks_far
) {
	const auto edge_zoomout_mult = looks_far ? 1.0f : 0.0f;
	const auto expansion = fog_of_war_settings::calc_camera_requested_expansion(camera_zoom, edge_zoomout_mult);
	const auto considered = fog_of_war.get_considered(expansion, edge_zoomout_mult);

	auto result = ai_field_of_view();
	result.half_size = considered.size * mult;

	if (considered.is_enabled()) {
		result.angle = considered.angle * mult;
	}

	return result;
}
