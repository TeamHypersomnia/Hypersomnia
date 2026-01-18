#pragma once
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/modes/ai/tasks/navigate_pathfinding.hpp"
#include "game/modes/ai/tasks/update_camp_twitch.hpp"
#include "game/modes/ai/tasks/should_helpers.hpp"

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
	- Current pathfinding state (if navigating to a target)
	- Camp twitching (if camping on a waypoint)
	- Defusing (no movement)
	
	Also outputs crosshair offset for aiming.
*/

template <typename CharacterHandle>
inline movement_direction_result calc_current_movement_direction(
	arena_mode_ai_state& ai_state,
	const vec2 character_pos,
	const cosmos_navmesh& navmesh,
	CharacterHandle character,
	const real32 dt,
	randomization& rng
) {
	movement_direction_result result;

	/*
		If defusing, don't move at all.
	*/
	if (ai_state.is_defusing) {
		return result;
	}

	/*
		If camping (patrolling with camp timer > 0), do camp twitching.
	*/
	if (::is_camping(ai_state)) {
		const auto twitch_dir = ::update_camp_twitch(ai_state, character_pos, dt, rng);
		result.direction = twitch_dir;
		/*
			Look in the direction of the waypoint transform while camping.
		*/
		result.crosshair_offset = ai_state.camp_look_direction * 200.0f;
		return result;
	}

	/*
		If pathfinding is active, use navigate_pathfinding for proper path following.
	*/
	if (ai_state.is_pathfinding_active()) {
		const auto nav_result = ::navigate_pathfinding(
			ai_state.pathfinding,
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
