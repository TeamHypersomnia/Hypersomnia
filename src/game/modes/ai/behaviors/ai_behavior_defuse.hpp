#pragma once
#include "augs/math/vec2.h"
#include "game/cosmos/entity_id.h"

/*
	Defuse behavior.
	Bot is tasked with defusing the planted bomb.
	
	State tracks whether we've started the defuse action.
*/

struct ai_behavior_defuse {
	// GEN INTROSPECTOR struct ai_behavior_defuse
	bool is_defusing = false;
	// END GEN INTROSPECTOR

	bool operator==(const ai_behavior_defuse&) const = default;

	/*
		Process defuse behavior for this frame.
		Handles path completion by starting defuse.
	*/
	template <typename CharacterHandle, typename Step>
	void process(
		CharacterHandle character_handle,
		Step& step,
		const vec2 character_pos,
		const entity_id bomb_entity,
		vec2& target_crosshair_offset,
		const bool pathfinding_just_completed
	);
};
