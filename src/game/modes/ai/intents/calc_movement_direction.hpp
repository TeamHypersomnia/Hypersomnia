#pragma once
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/detail/pathfinding/navigate_pathfinding.hpp"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/modes/ai/behaviors/ai_behavior_variant.hpp"
#include "game/modes/ai/intents/calc_movement_flags.hpp"

/*
	Result of movement direction calculation.
*/

struct movement_direction_result {
	std::optional<vec2> direction;
	vec2 crosshair_offset = vec2::zero;
	bool path_completed = false;
	bool is_navigating = false;
	bool can_sprint = false;  /* True when movement direction is mostly parallel to path direction (within ~15 degrees). */
};

/*
	Stateless calculation of the current movement direction.
	
	This function determines HOW the bot should move based on:
	- Current behavior type
	- Current pathfinding state (if navigating to a target)
	- Camp twitching result (from patrol's process())
	- Defusing (no movement)
	
	Also outputs crosshair offset for aiming.
	Uses std::visit on the behavior variant.
	
	NOTE: Camp twitch updates are now handled in ai_behavior_patrol::process().
	This function just reads the twitch_direction result.
*/

template <typename CharacterHandle>
inline movement_direction_result calc_current_movement_direction(
	const ai_behavior_variant& behavior,
	std::optional<ai_pathfinding_state>& pathfinding,
	const vec2 character_pos,
	const cosmos_navmesh& navmesh,
	CharacterHandle character,
	const real32 dt,
	const cosmos& cosm,
	const entity_id bomb_entity
) {
	movement_direction_result result;

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
		Check if camping (patrol with camp timer > 0) - use twitch direction from process().
	*/
	if (const auto* patrol = ::get_behavior_if<ai_behavior_patrol>(behavior)) {
		if (patrol->is_camping()) {
			result.direction = patrol->twitch_direction;
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
		const auto nav_result = ::navigate_pathfinding(
			pathfinding,
			character_pos,
			navmesh,
			character,
			dt
		);

		result.path_completed = nav_result.path_completed;
		result.is_navigating = nav_result.is_navigating;
		result.crosshair_offset = nav_result.crosshair_offset;
		result.can_sprint = nav_result.can_sprint;

		if (nav_result.is_navigating) {
			result.direction = nav_result.movement_direction;
		}
		else if (nav_result.path_completed) {
			/*
				Path completed - don't move, but set crosshair to face target direction.
			*/
			result.crosshair_offset = nav_result.crosshair_offset;
		}
	}

	return result;
}
