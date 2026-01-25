#pragma once
#include "game/cosmos/for_each_entity.h"
#include "game/detail/sentience/sentience_getters.h"
#include "game/enums/filters.h"
#include "game/modes/ai/ai_character_context.h"
#include "game/modes/ai/tasks/line_of_sight.hpp"

inline entity_id find_closest_enemy(
	const ai_character_context& ctx,
	const bool is_ffa,
	const bool is_camping = false
) {
	entity_id closest_enemy = entity_id::dead();
	float closest_distance = std::numeric_limits<float>::max();

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

			const auto target_pos = entity.get_logic_transform().pos;
			const auto offset = target_pos - ctx.character_pos;
			const auto distance = offset.length();

			/*
				Check if target is within field of view (using line_of_sight.hpp functions).
			*/
			if (!::is_within_fov(ctx.character_pos, target_pos, is_camping)) {
				return;
			}

			if (distance < closest_distance) {
				const auto direction_to_enemy = vec2(offset).normalize();
				const auto angle_to_enemy = current_aim_direction.degrees_between(direction_to_enemy);

				if (angle_to_enemy <= 70.0f) {
					/*
						Check line of sight using the helper function.
					*/
					if (::is_in_line_of_sight(ctx.character_pos, target_pos, ctx.physics, ctx.cosm, ctx.character_handle)) {
						closest_enemy = entity;
						closest_distance = distance;
					}
				}
			}
		}
	);

	return closest_enemy;
}
