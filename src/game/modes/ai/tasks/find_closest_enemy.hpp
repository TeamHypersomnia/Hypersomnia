#pragma once
#include "game/cosmos/for_each_entity.h"
#include "game/detail/sentience/sentience_getters.h"
#include "game/enums/filters.h"
#include "game/modes/ai/ai_character_context.h"
#include "game/modes/ai/tasks/line_of_sight.hpp"
#include "game/modes/ai/tasks/ai_field_of_view.h"

/*
	Find the closest visible enemy within the bot's field of view.
*/

inline entity_id find_closest_enemy(
	const ai_character_context& ctx,
	const bool is_ffa,
	const ai_field_of_view& fov
) {
	entity_id closest_enemy = entity_id::dead();
	float closest_distance = std::numeric_limits<float>::max();

	/*
		Right after spawning the crosshair can still be zero -
		it would be 90 degrees away from everything, so look where the body faces.
	*/
	const auto current_aim_direction = [&]() {
		if (auto crosshair = ctx.character_handle.find_crosshair()) {
			if (!crosshair->base_offset.is_zero()) {
				return vec2(crosshair->base_offset).normalize();
			}
		}

		return vec2::from_degrees(ctx.character_handle.get_logic_transform().rotation);
	}();

	const auto si = ctx.cosm.get_si();
	const auto filter = predefined_queries::line_of_sight();

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

			/*
				Check if target is within field of view first (early exit for efficiency).
			*/
			if (!fov.within_box(ctx.character_pos, target_pos)) {
				return;
			}

			const auto offset = target_pos - ctx.character_pos;
			const auto distance = offset.length();

			if (distance < closest_distance) {
				const auto direction_to_enemy = vec2(offset).normalize();

				if (fov.within_angle(current_aim_direction, direction_to_enemy)) {
					/*
						Check line of sight using the helper function.
					*/
					if (::los_to_any_vertices_of(entity, ctx.character_pos, ctx.physics, si, filter)) {
						closest_enemy = entity;
						closest_distance = distance;
					}
				}
			}
		}
	);

	return closest_enemy;
}
