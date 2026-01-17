#pragma once
#include "game/cosmos/cosmos.h"
#include "game/inferred_caches/physics_world_cache.h"
#include "game/enums/filters.h"
#include "augs/math/repro_math.h"

/*
	Field of view constants.
	Normal FOV: 1920x1080 (standard resolution)
	Extended FOV (camping): 2688x1560 (40% larger in each dimension)
*/
constexpr float AI_FOV_NORMAL_X = 1920.0f;
constexpr float AI_FOV_NORMAL_Y = 1080.0f;
constexpr float AI_FOV_EXTENDED_X = 2688.0f;
constexpr float AI_FOV_EXTENDED_Y = 1560.0f;

/*
	Check if a position is within line of sight from another position.
	Uses the physics system for raycasting.
*/

template <class E>
inline bool is_in_line_of_sight(
	const vec2 from_pos,
	const vec2 to_pos,
	const physics_world_cache& physics,
	const cosmos& cosm,
	const E& ignore_entity
) {
	const auto filter = predefined_queries::line_of_sight();
	const auto raycast = physics.ray_cast_px(
		cosm.get_si(),
		from_pos,
		to_pos,
		filter,
		ignore_entity
	);

	return !raycast.hit;
}

/*
	Check if a position is within the bot's field of view.
*/

inline bool is_within_fov(
	const vec2 from_pos,
	const vec2 target_pos,
	const bool camping
) {
	const auto offset = target_pos - from_pos;
	const float fov_x = camping ? AI_FOV_EXTENDED_X : AI_FOV_NORMAL_X;
	const float fov_y = camping ? AI_FOV_EXTENDED_Y : AI_FOV_NORMAL_Y;

	return repro::fabs(offset.x) < fov_x && repro::fabs(offset.y) < fov_y;
}
