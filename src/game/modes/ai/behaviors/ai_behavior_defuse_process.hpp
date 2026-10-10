#pragma once
#include "game/modes/ai/behaviors/ai_behavior_defuse.hpp"
#include "game/modes/ai/behaviors/ai_behavior_process_ctx.hpp"
#include "game/components/sentience_component.h"
#include "game/components/hand_fuse_component.h"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/enums/requested_interaction_type.h"
#include "game/detail/hand_fuse_logic.h"
#include "game/cosmos/make_physics_path_hints.h"

/*
	Implementation of ai_behavior_defuse::process().
	
	NOTE: Weapon holstering for defuse is handled statelessly via
	should_holster_weapons() which checks is_defusing.
	
	NOTE: Crosshair offset and requested_interactions are now calculated statelessly
	via calc_movement_and_crosshair() and calc_requested_interaction().
*/

/*
	Same check as the one that lets the defuse begin in query_use_interaction:
	the bomb must overlap the character's interaction hitbox (a sector in front of it).
*/

template <class C, class B>
inline bool is_bomb_within_defuse_reach(const C& character_handle, const B& bomb_handle) {
	auto in_reach = false;

	bomb_handle.template dispatch_on_having_all<components::hand_fuse>([&](const auto& typed_bomb) {
		in_reach = stepless_fuse_logic_provider(typed_bomb).defusing_character_in_range(character_handle);
	});

	return in_reach;
}

inline void ai_behavior_defuse::process(ai_behavior_process_ctx& ctx) {
	const auto& cosm = ctx.cosm;
	const auto character_handle = cosm[ctx.controlled_character_id];
	const auto bomb_handle = cosm[ctx.bomb_entity];

	if (!character_handle.alive() || !bomb_handle.alive()) {
		is_defusing = false;
		is_approaching_directly = false;
		return;
	}

	const auto bomb_pos = bomb_handle.get_logic_transform().pos;

	/*
		The path ends on the closest walkable cell, which might still be out of reach
		if the bomb lies on unwalkable cells, e.g. in a nook between walls.
		Cover the rest by walking straight at it.
		If a wall gets between the bot and the bomb, go back to pathfinding.
	*/
	if (ctx.navigation_just_completed) {
		is_approaching_directly = true;
	}

	if (is_approaching_directly) {
		const auto physics_hints = ::make_physics_path_hints(cosm);

		if (!physics_hints.has_line_of_sight(ctx.character_pos, bomb_pos)) {
			is_approaching_directly = false;
		}
	}

	/*
		Defuse only while the bomb is actually within reach.
		Otherwise the bot would holster and stand still out of reach,
		then flip back to its weapon and the path every other frame.

		If someone else is already defusing, keep waiting by the bomb
		with bare hands to take over as soon as they stop.
	*/
	const bool in_reach = ::is_bomb_within_defuse_reach(character_handle, bomb_handle);

	if (!is_defusing && in_reach) {
		AI_LOG("Bomb within reach - starting defuse");
	}

	is_defusing = in_reach;
}
