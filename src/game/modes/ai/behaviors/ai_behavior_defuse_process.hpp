#pragma once
#include "game/modes/ai/behaviors/ai_behavior_defuse.hpp"
#include "game/modes/ai/behaviors/ai_behavior_process_ctx.hpp"
#include "game/components/sentience_component.h"
#include "game/detail/inventory/wielding_setup.hpp"
#include "game/detail/inventory/perform_wielding.hpp"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"

/*
	Implementation of ai_behavior_defuse::process().
*/

inline void ai_behavior_defuse::process(ai_behavior_process_ctx& ctx) {
	auto& cosm = ctx.cosm;
	const auto& step = ctx.step;
	const auto& character_pos = ctx.character_pos;
	const auto bomb_entity = ctx.bomb_entity;
	auto& target_crosshair_offset = ctx.ai_state.target_crosshair_offset;
	const bool pathfinding_just_completed = ctx.pathfinding_just_completed;

	const auto character_handle = cosm[ctx.controlled_character_id];

	if (!character_handle.alive()) {
		return;
	}

	/*
		Handle path completion - start defusing.
	*/
	if (pathfinding_just_completed && !is_defusing) {
		AI_LOG("Reached bomb - starting defuse");
		is_defusing = true;

		const auto current_wielding = wielding_setup::from_current(character_handle);

		if (!current_wielding.is_bare_hands(cosm)) {
			::perform_wielding(step, character_handle, wielding_setup::bare_hands());
		}

		if (auto* sentience = character_handle.find<components::sentience>()) {
			sentience->is_requesting_interaction = true;
		}
	}

	/*
		If defusing, aim at bomb and request interaction.
	*/
	if (is_defusing && bomb_entity.is_set()) {
		const auto bomb_handle = cosm[bomb_entity];

		if (bomb_handle.alive()) {
			const auto bomb_pos = bomb_handle.get_logic_transform().pos;
			target_crosshair_offset = bomb_pos - character_pos;

			if (auto* sentience = character_handle.find<components::sentience>()) {
				sentience->is_requesting_interaction = true;
			}
		}
	}
}
