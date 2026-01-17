#pragma once
#include "game/detail/pathfinding.h"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/modes/ai/tasks/ai_pathfinding.hpp"
#include "game/components/movement_component.h"
#include "game/components/crosshair_component.h"

/*
	Unified AI pathfinding navigation task.
	
	Handles all path following logic in one reusable function:
	- Advances path when bot reaches cell centers
	- Checks for path deviation and reroutes if needed
	- Sets movement flags from pathfinding direction
	- Updates crosshair offset with look-ahead smoothing
	- Debug draws the path
	
	Returns true if the path is still being followed.
	Returns false if path was completed or pathfinding state should be cleared.
*/

struct navigate_pathfinding_result {
	bool is_navigating = false;
	bool path_completed = false;
};

template <typename CharacterHandle>
inline navigate_pathfinding_result navigate_pathfinding(
	std::optional<ai_pathfinding_state>& pathfinding_opt,
	const vec2 bot_pos,
	const cosmos_navmesh& navmesh,
	CharacterHandle character,
	vec2& target_crosshair_offset,
	const real32 dt
) {
	navigate_pathfinding_result result;

	if (!pathfinding_opt.has_value()) {
		return result;
	}

	auto& pathfinding = *pathfinding_opt;

	/*
		Advance along path and check for deviation.
	*/
	bool path_completed = false;
	bool is_inert = false;

	if (auto* movement = character.template find<components::movement>()) {
		if (movement->portal_inertia_ms > 50.0f) {
			is_inert = true;
		}
	}

	if (const auto rigid_body = character.template find<components::rigid_body>()) {
		if (rigid_body.is_inside_portal()) {
			is_inert = true;
		}
	}

	if (!is_inert) {
		::advance_path_if_cell_reached(pathfinding, bot_pos, navmesh, path_completed);
	}

	if (path_completed) {
		pathfinding_opt.reset();
		result.path_completed = true;
		return result;
	}

	if (!is_inert) {
		::check_path_deviation(pathfinding, bot_pos, navmesh, nullptr);
	}

	/*
		Get movement direction from pathfinding.
		Also sets target_crosshair_offset as an output parameter (un-normalized).
	*/
	const auto movement_dir = ::get_pathfinding_movement_direction(
		pathfinding,
		bot_pos,
		navmesh,
		target_crosshair_offset,
		dt
	);

	if (!movement_dir.has_value()) {
		return result;
	}

	/*
		Apply movement flags using normalized direction.
	*/
	if (auto* movement = character.template find<components::movement>()) {
		movement->flags.set_from_closest_direction(*movement_dir);
	}

	/*
		Debug draw the path.
	*/
	::debug_draw_pathfinding(pathfinding_opt, bot_pos, navmesh);

	result.is_navigating = true;
	return result;
}
