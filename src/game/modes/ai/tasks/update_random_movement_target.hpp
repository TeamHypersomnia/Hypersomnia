#pragma once
#include "augs/misc/randomization.h"
#include "game/enums/filters.h"
#include "game/modes/ai/ai_character_context.h"
#include "game/modes/arena_mode_ai_structs.h"
#include "game/inferred_caches/physics_world_cache.h"

inline void update_random_movement_target(
	const ai_character_context& ctx,
	const float dt_secs
) {
	ctx.ai_state.movement_timer_remaining -= dt_secs;

	if (ctx.ai_state.movement_timer_remaining <= 0.0f) {
		const auto seed = ctx.cosm.get_rng_seed_for(ctx.character_handle);
		auto rng = randomization(seed);

		const auto random_direction = [&]() {
			if (ctx.ai_state.chase_timeout > 0.0f) {
				const auto rot = rng.randval(-135, 135);
				return (ctx.character_pos - ctx.ai_state.last_target_position).normalize().rotate(rot);
			}

			return rng.random_point_on_unit_circle<real32>();
		}();

		const auto raycast_distance = 1500.0f;
		const auto raycast_end = ctx.character_pos + random_direction * raycast_distance;
		const auto filter = predefined_queries::pathfinding();

		const auto raycast = ctx.physics.ray_cast_px(
			ctx.cosm.get_si(),
			ctx.character_pos,
			raycast_end,
			filter,
			ctx.character_handle
		);

		ctx.ai_state.random_movement_target = raycast.hit ? raycast.intersection : raycast_end;
		ctx.ai_state.movement_duration_secs = rng.randval(1.0f, 3.0f);
		ctx.ai_state.movement_timer_remaining = ctx.ai_state.movement_duration_secs;
	}
}
