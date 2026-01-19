#pragma once
#include "augs/misc/enum/enum_bitset.h"
#include "game/modes/ai/behaviors/ai_behavior_variant.hpp"
#include "game/enums/requested_interaction_type.h"

/*
	Stateless calculation of requested interactions bitset.
	
	This function determines WHAT interactions the bot is requesting based on:
	- Current behavior type
	- Defusing state (if in defuse behavior)
	
	AI defusing only sets the DEFUSE bit, not PICK_UP_ITEMS.
	This prevents bots from picking up items while defusing.
*/

inline uint8_t calc_requested_interaction(const ai_behavior_variant& behavior) {
	uint8_t result = 0u;

	if (const auto* defuse = ::get_behavior_if<ai_behavior_defuse>(behavior)) {
		if (defuse->is_defusing) {
			augs::set_bit(result, requested_interaction_type::DEFUSE);
		}
	}

	return result;
}
