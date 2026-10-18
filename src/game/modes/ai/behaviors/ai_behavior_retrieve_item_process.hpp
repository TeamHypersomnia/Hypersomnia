#pragma once
#include "game/modes/ai/behaviors/ai_behavior_retrieve_item.hpp"
#include "game/modes/ai/behaviors/ai_behavior_process_ctx.hpp"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/detail/item_pickup_process.hpp"

/*
	Once reached, the item is picked up. If it still lies there (e.g. it could not be picked up from where the path ended),
	the bot gives up on it, so that it doesn't walk to it again and again.
*/

inline void ai_behavior_retrieve_item::process(ai_behavior_process_ctx& ctx) {
	if (!ctx.navigation_just_completed) {
		return;
	}

	auto& ai_state = ctx.ai_state;
	const auto item = ai_state.item_to_retrieve;

	if (ctx.cosm[item].alive()) {
		item_pickup { item }.process(ctx.step, ctx.controlled_character_id);
	}

	if (const auto still_there = ctx.cosm[item]; still_there.alive() && still_there.get_owning_transfer_capability().dead()) {
		ai_state.unreachable_item = item;
	}

	ai_state.item_to_retrieve = entity_id();
}
