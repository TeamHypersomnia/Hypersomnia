#pragma once
#include <optional>
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/detail/pathfinding/navigate_pathfinding.hpp"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/modes/ai/behaviors/ai_behavior_variant.hpp"
#include "game/modes/ai/intents/calc_movement_flags.hpp"
#include "game/modes/ai/tasks/estimate_aiming_target.hpp"

/*
	Stateless calculation of the current movement direction and crosshair aim.
	
	This function determines HOW the bot should move based on:
	- Current behavior type
	- Current pathfinding state (if navigating to a target)
	- Camp twitching result (from patrol's process())
	- Defusing (no movement)
	
	Also outputs crosshair offset for aiming based on:
	- Combat targets (when target_acquired is true)
	- Bomb location (when defusing)
	- Plant target (when planting)
	- Waypoint direction (when camping)
	- Path direction (when navigating)
	
	Uses std::visit on the behavior variant.
	
	NOTE: Camp twitch updates are now handled in ai_behavior_patrol::process().
	This function just reads the twitch_direction result.
	
	Returns navigate_pathfinding_result which contains:
	- is_navigating: currently following a path
	- path_completed: destination reached
	- can_sprint: movement mostly parallel to path
	- nearing_end: close to destination (use for holstering)
	- movement_direction: direction to move
	- crosshair_offset: where to aim
	
	target_enemy_pos: The position to aim at (last known or current enemy position).
	target_enemy_velocity: Optional velocity for aim prediction. If set, predict enemy
	movement; if not set (wall penetration), aim at position directly.
*/

template <typename CharacterHandle>
inline navigate_pathfinding_result calc_movement_and_crosshair(
	const ai_behavior_variant& behavior,
	std::optional<ai_pathfinding_state>& pathfinding,
	const vec2 character_pos,
	const cosmos_navmesh& navmesh,
	CharacterHandle character,
	const real32 dt,
	const cosmos& cosm,
	const entity_id bomb_entity,
	const bool target_acquired,
	const vec2 target_enemy_pos,
	const std::optional<vec2> target_enemy_velocity = std::nullopt
) {
	navigate_pathfinding_result result;

	/*
		Check if defusing - don't move, but aim at the bomb statelessly.
	*/
	if (const auto* defuse = ::get_behavior_if<ai_behavior_defuse>(behavior)) {
		if (defuse->is_defusing) {
			/*
				Statelessly calculate crosshair offset to aim at the bomb.
			*/
			if (bomb_entity.is_set()) {
				const auto bomb_handle = cosm[bomb_entity];

				if (bomb_handle.alive()) {
					const auto bomb_pos = bomb_handle.get_logic_transform().pos;
					result.crosshair_offset = bomb_pos - character_pos;
				}
			}

			return result;
		}
	}

	/*
		Check if planting - don't move, aim at cached plant target.
	*/
	if (const auto* plant = ::get_behavior_if<ai_behavior_plant>(behavior)) {
		if (plant->is_planting) {
			/*
				Statelessly calculate crosshair offset to aim at plant target.
			*/
			if (plant->cached_plant_target.has_value()) {
				const auto target_pos = plant->cached_plant_target->pos;
				result.crosshair_offset = target_pos - character_pos;
			}
			else {
				/*
					Fallback: look forward.
				*/
				result.crosshair_offset = vec2(1.0f, 0.0f) * 200.0f;
			}

			return result;
		}
	}

	/*
		Check if camping (patrol with camp timer > 0) - use twitch direction from process().
	*/
	if (const auto* patrol = ::get_behavior_if<ai_behavior_patrol>(behavior)) {
		if (patrol->is_camping()) {
			result.movement_direction = patrol->twitch_direction;
			/*
				Look in the direction of the waypoint transform while camping.
			*/
			result.crosshair_offset = patrol->camp_look_direction * 200.0f;
			return result;
		}
	}

	/*
		If pathfinding is active, use navigate_pathfinding for proper path following.
	*/
	if (pathfinding.has_value()) {
		result = ::navigate_pathfinding(
			pathfinding,
			character_pos,
			navmesh,
			character,
			dt
		);
	}

	/*
		If has combat target, aim at the target position.
		If velocity is provided, predict enemy movement using estimate_aiming_target.
		Otherwise (wall penetration), aim at the position directly.
	*/
	if (target_acquired) {
		if (target_enemy_velocity.has_value()) {
			/*
				We see the enemy - predict their position based on velocity.
			*/
			const auto predicted_pos = ::estimate_aiming_target(character, target_enemy_pos, *target_enemy_velocity);
			result.crosshair_offset = predicted_pos - character_pos;
		}
		else {
			/*
				Wall penetration case: aim at last known position directly.
			*/
			result.crosshair_offset = target_enemy_pos - character_pos;
		}
	}

	return result;
}
