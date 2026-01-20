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

/*
	Stateless calculation of hand_flags for planting.
	
	When is_planting is true, the bot should be holding the trigger
	to initiate bomb planting (arming_requested = true in hand_fuse_logic).
	
	Returns a pair of booleans for hand_flags[0] and hand_flags[1].
*/

struct hand_flags_result {
	bool hand_flag_0 = false;
	bool hand_flag_1 = false;
	bool should_apply = false;
};

inline hand_flags_result calc_hand_flags(const ai_behavior_variant& behavior) {
	hand_flags_result result;

	if (const auto* plant = ::get_behavior_if<ai_behavior_plant>(behavior)) {
		if (plant->is_planting) {
			/*
				Set hand_flags to true to trigger arming_requested on the bomb.
			*/
			result.hand_flag_0 = true;
			result.hand_flag_1 = true;
			result.should_apply = true;
		}
	}

	return result;
}
