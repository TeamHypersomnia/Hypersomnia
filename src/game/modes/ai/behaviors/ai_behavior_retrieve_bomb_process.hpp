#pragma once
#include "game/modes/ai/behaviors/ai_behavior_retrieve_bomb.hpp"
#include "game/modes/ai/behaviors/ai_behavior_process_ctx.hpp"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/detail/use_interaction_logic.h"

/*
	Implementation of ai_behavior_retrieve_bomb::process().
	
	When pathfinding completes, force item_pickup { bomb }.process(step, character)
	to pick up the bomb. The behavior tree will automatically re-evaluate
	to a different behavior once the bomb is picked up.
*/

inline void ai_behavior_retrieve_bomb::process(ai_behavior_process_ctx& ctx) {
	const bool pathfinding_just_completed = ctx.pathfinding_just_completed;
	const auto bomb_entity = ctx.bomb_entity;

	if (!pathfinding_just_completed) {
		return;
	}

	if (const auto bomb_handle = ctx.cosm[bomb_entity]) {
		AI_LOG("Reached bomb - picking up");

		item_pickup { bomb_entity }.process(ctx.step, ctx.controlled_character_id);
	}
}
