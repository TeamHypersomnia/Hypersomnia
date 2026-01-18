#pragma once
#include "game/modes/ai/behaviors/ai_behavior_defuse.hpp"
#include "game/components/sentience_component.h"
#include "game/detail/inventory/wielding_setup.hpp"
#include "game/detail/inventory/perform_wielding.hpp"
#include "game/modes/ai/arena_mode_ai_structs.h"

/*
	Implementation of ai_behavior_defuse::process().
	Separated into a .hpp file for proper template instantiation.
*/

template <typename CharacterHandle, typename Step>
void ai_behavior_defuse::process(
	CharacterHandle character_handle,
	Step& step,
	const vec2 character_pos,
	const entity_id bomb_entity,
	vec2& target_crosshair_offset,
	const bool pathfinding_just_completed
) {
	auto& cosm = character_handle.get_cosmos();

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

		if (auto* sentience = character_handle.template find<components::sentience>()) {
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

			if (auto* sentience = character_handle.template find<components::sentience>()) {
				sentience->is_requesting_interaction = true;
			}
		}
	}
}
