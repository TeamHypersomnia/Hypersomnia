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
	- Calculates movement direction (does NOT apply it - caller must do that)
	- Updates crosshair offset with look-ahead smoothing
	- Debug draws the path
	
	Returns the movement direction if the path is still being followed.
	The caller is responsible for applying the movement direction.
*/

struct navigate_pathfinding_result {
	bool is_navigating = false;
	bool path_completed = false;
	std::optional<vec2> movement_direction;
	vec2 crosshair_offset = vec2::zero;
};

template <typename CharacterHandle>
inline navigate_pathfinding_result navigate_pathfinding(
	std::optional<ai_pathfinding_state>& pathfinding_opt,
	const vec2 bot_pos,
	const cosmos_navmesh& navmesh,
	CharacterHandle character,
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

	/*
		Check for exact destination mode: path is only completed when
		the bot reaches the exact target position, not just the target cell center.
	*/
	if (path_completed && pathfinding.exact_destination) {
		const float dist_to_exact = (bot_pos - pathfinding.target_position).length();
		constexpr float EXACT_REACH_EPSILON = 30.0f;

		if (dist_to_exact > EXACT_REACH_EPSILON) {
			/*
				Not at exact position yet - continue navigating directly to target.
			*/
			path_completed = false;
			const auto dir = pathfinding.target_position - bot_pos;
			result.crosshair_offset = dir;
			result.movement_direction = dir.normalize();
			result.is_navigating = true;

			::debug_draw_pathfinding(pathfinding_opt, bot_pos, navmesh);
			return result;
		}
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
		Also calculates crosshair offset (un-normalized).
	*/
	const auto movement_dir = ::get_pathfinding_movement_direction(
		pathfinding,
		bot_pos,
		navmesh,
		result.crosshair_offset,
		dt
	);

	if (!movement_dir.has_value()) {
		return result;
	}

	result.movement_direction = *movement_dir;

	/*
		Debug draw the path.
	*/
	::debug_draw_pathfinding(pathfinding_opt, bot_pos, navmesh);

	result.is_navigating = true;
	return result;
}
