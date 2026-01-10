#pragma once
#include "game/cosmos/for_each_entity.h"
#include "game/detail/sentience/sentience_getters.h"
#include "game/enums/filters.h"
#include "game/modes/ai/ai_character_context.h"
#include "game/inferred_caches/physics_world_cache.h"

inline entity_id find_closest_enemy(
	const ai_character_context& ctx,
	const bool is_ffa
) {
	entity_id closest_enemy = entity_id::dead();
	float closest_distance = std::numeric_limits<float>::max();
	const auto filter = predefined_queries::line_of_sight();

	const auto current_aim_direction = [&]() {
		if (auto crosshair = ctx.character_handle.find_crosshair()) {
			return vec2(crosshair->base_offset).normalize();
		}

		return vec2::zero;
	}();

	ctx.cosm.for_each_having<components::sentience>(
		[&](const auto& entity) {
			if (entity == ctx.character_handle || !entity.alive() || !::sentient_and_conscious(entity)) {
				return;
			}

			if (!is_ffa) {
				const auto bot_faction = ctx.character_handle.get_official_faction();
				const auto target_faction = entity.get_official_faction();

				if (bot_faction == target_faction) {
					return;
				}
			}

			const auto offset = entity.get_logic_transform().pos - ctx.character_pos;
			const auto distance = offset.length();

			if (distance < closest_distance && repro::fabs(offset.y) < 1080.0f && repro::fabs(offset.x) < 1920.0f) {
				const auto direction_to_enemy = vec2(offset).normalize();
				const auto angle_to_enemy = current_aim_direction.degrees_between(direction_to_enemy);

				if (angle_to_enemy <= 70.0f) {
					const auto raycast = ctx.physics.ray_cast_px(
						ctx.cosm.get_si(),
						ctx.character_pos,
						entity.get_logic_transform().pos,
						filter,
						ctx.character_handle
					);

					if (!raycast.hit) {
						closest_enemy = entity;
						closest_distance = distance;
					}
				}
			}
		}
	);

	return closest_enemy;
}
