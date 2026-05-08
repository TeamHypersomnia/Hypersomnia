#pragma once
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/components/sentience_component.h"
#include "game/components/gun_component.h"
#include "game/components/melee_component.h"
#include "game/detail/gun/gun_math.h"
#include "game/modes/ai/behaviors/ai_behavior_variant.hpp"

/*
	Returns the bot_aim_radius_to_shoot for the first wielded gun found on the
	character, falling back to 25.0f when no gun is wielded.
*/
template <class CharacterHandle>
real32 calc_aim_radius_to_shoot(const CharacterHandle& character_handle) {
	const auto& cosm = character_handle.get_cosmos();

	for (const auto& item_id : character_handle.get_wielded_items()) {
		if (const auto gun_def = cosm[item_id].template find<invariants::gun>()) {
			return gun_def->bot_aim_radius_to_shoot;
		}
	}

	return 25.0f;
}

/*
	Stateless calculation of hand_flags.

	This function determines whether the bot should be holding the trigger based on:
	- Current behavior type (planting, combat, etc.)
	- Combat target state (has target, is aiming correctly)
	- Frozen state

	Always returns an up-to-date state for the hands.

	aim_pos: The position to aim at (last known or current enemy position).

	melee_reaction_timer: persistent gating timer for melee swings.
	  - Builds up while the bot stays within the secondary bot_attack_range of aim_pos.
	  - Drains (clamped to 0) while outside, so a brief out-of-range frame
	    doesn't reset commitment.
	  - Reset to 0 the moment a swing is committed.
	melee_reaction_threshold: in-range time required before the bot can swing.
*/

struct hand_flags_result {
	bool hand_flag_0 = false;
	bool hand_flag_1 = false;
};

template <typename CharacterHandle>
inline hand_flags_result calc_hand_flags(
	const ai_behavior_variant& behavior,
	const bool target_acquired,
	const vec2 aim_pos,
	CharacterHandle character_handle,
	real32& melee_reaction_timer,
	const real32 melee_reaction_threshold,
	const real32 dt_secs
) {
	hand_flags_result result;

	const auto drain_melee_timer = [&]() {
		melee_reaction_timer = std::max(0.0f, melee_reaction_timer - dt_secs);
	};

	/*
		If planting, set hand_flags to trigger arming_requested on the bomb.
	*/
	if (const auto* plant = ::get_behavior_if<ai_behavior_plant>(behavior)) {
		if (plant->is_planting) {
			result.hand_flag_0 = true;
			result.hand_flag_1 = true;
			drain_melee_timer();
			return result;
		}
	}

	/*
		Combat target: trigger if aiming correctly.
		Instead of comparing angles, project the enemy position onto the bullet
		trajectory and check if the perpendicular (lateral) distance from the
		enemy to the aimed ray is within the threshold.

		If a melee weapon is wielded instead, decide which attack to fire based
		on the distance to the target rather than aiming geometry. The bot must
		also have stayed in range long enough (melee_reaction_timer) to commit.
	*/
	bool melee_path_handled = false;

	if (const auto* combat = ::get_behavior_if<ai_behavior_combat>(behavior)) {
		if (target_acquired) {
			const auto& cosm = character_handle.get_cosmos();

			for (const auto& item_id : character_handle.get_wielded_items()) {
				const auto wielded_handle = cosm[item_id];

				if (const auto melee_def = wielded_handle.template find<invariants::melee>()) {
					melee_path_handled = true;

					const auto primary_range = melee_def->actions[weapon_action_type::PRIMARY].bot_attack_range;
					const auto secondary_range = melee_def->actions[weapon_action_type::SECONDARY].bot_attack_range;

					const auto dist = (aim_pos - character_handle.get_logic_transform().pos).length();

					if (dist <= secondary_range) {
						melee_reaction_timer += dt_secs;
					}
					else {
						drain_melee_timer();
					}

					if (melee_reaction_timer >= melee_reaction_threshold) {
						if (dist < primary_range) {
							result.hand_flag_0 = true;
							melee_reaction_timer = 0.0f;
						}
						else if (dist < secondary_range) {
							result.hand_flag_1 = true;
							melee_reaction_timer = 0.0f;
						}
					}

					break;
				}

				if (!wielded_handle.template has<invariants::gun>()) {
					continue;
				}

				const auto gun_transform = wielded_handle.get_logic_transform();
				const auto barrel_center = ::calc_barrel_center(wielded_handle, gun_transform);
				const auto muzzle = ::calc_muzzle_transform(wielded_handle, gun_transform).pos;

				/*
					Derive the actual bullet trajectory direction from the weapon geometry,
					the same way draw_crosshair_lasers.cpp does it for the laser sight.
				*/
				const auto ray_dir = vec2(muzzle - barrel_center).normalize();
				const auto to_enemy = aim_pos - muzzle;

				/*
					Project the enemy vector onto the aim ray and compute the
					perpendicular (lateral) offset from that ray.
				*/
				const auto proj_len = to_enemy.dot(ray_dir);
				const auto lateral_dist = (to_enemy - ray_dir * proj_len).length();

				if (proj_len > 0.0f && lateral_dist <= ::calc_aim_radius_to_shoot(character_handle)) {
					result.hand_flag_0 = true;
					result.hand_flag_1 = true;
				}

				break;
			}
		}
	}

	if (!melee_path_handled) {
		drain_melee_timer();
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
