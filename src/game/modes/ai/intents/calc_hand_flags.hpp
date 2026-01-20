#pragma once
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/components/sentience_component.h"
#include "game/modes/ai/behaviors/ai_behavior_variant.hpp"

/*
	Stateless calculation of hand_flags.
	
	This function determines whether the bot should be holding the trigger based on:
	- Current behavior type (planting, combat, etc.)
	- Combat target state (has target, is aiming correctly)
	- Frozen state
	
	Always returns an up-to-date state for the hands.
*/

struct hand_flags_result {
	bool hand_flag_0 = false;
	bool hand_flag_1 = false;
};

template <typename CharacterHandle>
inline hand_flags_result calc_hand_flags(
	const ai_behavior_variant& behavior,
	const bool has_target,
	const entity_id closest_enemy,
	const vec2 character_pos,
	const cosmos& cosm,
	CharacterHandle character_handle
) {
	hand_flags_result result;

	/*
		If planting, set hand_flags to trigger arming_requested on the bomb.
	*/
	if (const auto* plant = ::get_behavior_if<ai_behavior_plant>(behavior)) {
		if (plant->is_planting) {
			result.hand_flag_0 = true;
			result.hand_flag_1 = true;
			return result;
		}
	}

	/*
		Combat target: trigger if aiming correctly.
	*/
	if (has_target) {
		const auto target_pos = cosm[closest_enemy].get_logic_transform().pos;
		const auto aim_direction = target_pos - character_pos;

		if (auto crosshair = character_handle.find_crosshair()) {
			const auto current_aim = vec2(crosshair->base_offset).normalize();
			const auto target_aim = vec2(aim_direction).normalize();
			const auto angle_diff = current_aim.degrees_between(target_aim);

			if (angle_diff <= 25.0f) {
				result.hand_flag_0 = true;
				result.hand_flag_1 = true;
			}
		}
	}

	/*
		If frozen, don't trigger.
	*/
	if (character_handle.is_frozen()) {
		result.hand_flag_0 = false;
		result.hand_flag_1 = false;
	}

	return result;
}
