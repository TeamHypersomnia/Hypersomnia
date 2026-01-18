#pragma once
#include "game/detail/pathfinding.h"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/detail/pathfinding/path_helpers.hpp"
#include "game/detail/pathfinding/advance_path_if_cell_reached.hpp"
#include "game/detail/pathfinding/check_path_deviation.hpp"
#include "game/detail/pathfinding/get_pathfinding_movement_direction.hpp"
#include "game/detail/pathfinding/start_pathfinding_to.hpp"
#include "game/detail/pathfinding/debug_draw_pathfinding.hpp"
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
	bool path_completed = false;  /* True when destination was reached and pathfinding was cleared. */
	bool can_sprint = false;      /* True when movement direction is mostly parallel to path direction (within ~15 degrees). */
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
	bool cell_path_completed = false;
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
		::advance_path_if_cell_reached(pathfinding, bot_pos, navmesh, cell_path_completed);
	}

	/*
		Check for exact destination mode: path is only completed when
		the bot reaches the exact target position, not just the target cell center.
	*/
	if (cell_path_completed) {
		if (pathfinding.exact_destination) {
			const auto target_pos = pathfinding.target_position();
			const float dist_to_exact = (bot_pos - target_pos).length();
			constexpr float EXACT_REACH_EPSILON = 30.0f;

			if (dist_to_exact > EXACT_REACH_EPSILON) {
				/*
					Not at exact position yet - continue navigating directly to target.
					
					NOTE: We delegate crosshair easing to get_pathfinding_movement_direction
					which handles it continuously from the final cell center to the exact destination.
				*/
				const auto dir_result = ::get_pathfinding_movement_direction(
					pathfinding,
					bot_pos,
					navmesh,
					dt
				);
				
				result.movement_direction = dir_result.movement_direction;
				result.crosshair_offset = dir_result.crosshair_offset;
				result.is_navigating = dir_result.has_direction;

				::debug_draw_pathfinding(pathfinding_opt, bot_pos, navmesh);
				return result;
			}
			else {
				/* Reached exact destination - mark as completed. */
				result.path_completed = true;
				result.crosshair_offset = pathfinding.target_transform.get_direction() * 200.0f;
				pathfinding_opt.reset();
				return result;
			}
		}
		else {
			/* Non-exact mode - path completed at cell center. */
			result.path_completed = true;
			pathfinding_opt.reset();
			return result;
		}
	}

	if (!is_inert) {
		::check_path_deviation(pathfinding, bot_pos, navmesh, nullptr);
	}

	/*
		Check if we're in the "post-cell-path" phase for exact destination mode.
		This happens when the cell path is complete but we haven't reached
		the exact destination yet.
	*/
	const auto current_target = ::get_current_path_target(pathfinding, navmesh);
	
	if (!current_target.has_value() && pathfinding.exact_destination) {
		/*
			Cell path is complete. Check if we've reached the exact destination.
		*/
		const auto target_pos = pathfinding.target_position();
		const float dist_to_exact = (bot_pos - target_pos).length();
		constexpr float EXACT_REACH_EPSILON = 30.0f;
		
		if (dist_to_exact <= EXACT_REACH_EPSILON) {
			/* Reached exact destination - mark as completed. */
			result.path_completed = true;
			result.crosshair_offset = pathfinding.target_transform.get_direction() * 200.0f;
			pathfinding_opt.reset();
			return result;
		}
		
		/* Not at exact position yet - continue navigating directly to target. */
	}

	/*
		Get movement direction from pathfinding.
		Also calculates crosshair offset (un-normalized).
	*/
	const auto dir_result = ::get_pathfinding_movement_direction(
		pathfinding,
		bot_pos,
		navmesh,
		dt
	);

	if (!dir_result.has_direction) {
		return result;
	}

	result.movement_direction = dir_result.movement_direction;
	result.crosshair_offset = dir_result.crosshair_offset;

	/*
		Debug draw the path.
	*/
	::debug_draw_pathfinding(pathfinding_opt, bot_pos, navmesh);

	result.is_navigating = true;
	
	/*
		Calculate can_sprint: movement direction should be mostly parallel
		to the pathfinding direction (within ~15 degrees in the same direction).
		
		We check:
		1. Movement direction exists
		2. Crosshair offset is valid (non-zero)
		3. Angle between movement direction and crosshair direction is small (~15 degrees)
		4. They're pointing in the same direction (positive dot product)
	*/
	if (result.movement_direction.has_value()) {
		const auto move_dir = *result.movement_direction;
		const auto crosshair_dir = result.crosshair_offset;
		
		if (crosshair_dir.is_nonzero() && move_dir.is_nonzero()) {
			const auto crosshair_normalized = vec2(crosshair_dir).normalize();
			const auto move_normalized = vec2(move_dir).normalize();
			
			/*
				Check angle using degrees_between which returns 0-180.
				We want them to be within ~15 degrees of each other in the same direction.
			*/
			const auto angle_between = move_normalized.degrees_between(crosshair_normalized);
			constexpr float MAX_SPRINT_ANGLE = 15.0f;
			
			if (angle_between <= MAX_SPRINT_ANGLE) {
				result.can_sprint = true;
			}
		}
	}
	
	return result;
}
