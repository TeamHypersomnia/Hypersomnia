#pragma once
#include "game/detail/pathfinding/pathfinding.h"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/detail/path_navigation/path_helpers.hpp"
#include "game/detail/path_navigation/advance_path_if_cell_reached.hpp"
#include "game/detail/path_navigation/check_path_deviation.hpp"
#include "game/detail/path_navigation/get_navigation_movement_direction.hpp"
#include "game/detail/path_navigation/start_navigating_to.hpp"
#include "game/detail/path_navigation/debug_draw_path_navigation.hpp"
#include "game/components/movement_component.h"
#include "game/components/crosshair_component.h"

/*
	Unified AI path navigation task.
	
	Handles all path following logic in one reusable function:
	- Advances path when bot reaches cell centers
	- Checks for path deviation and reroutes if needed
	- Calculates movement direction (does NOT apply it - caller must do that)
	- Updates crosshair offset with look-ahead smoothing
	- Debug draws the path
	
	Returns the movement direction if the path is still being followed.
	The caller is responsible for applying the movement direction.
*/

struct navigate_path_result {
	bool is_navigating = false;
	bool path_completed = false;  /* True when destination was reached and navigation was cleared. */
	bool can_sprint = false;      /* True when movement direction is mostly parallel to path direction (within ~15 degrees). */
	bool nearing_end = false;     /* Will holster in some situations like during defuse. */
	std::optional<vec2> movement_direction;
	vec2 crosshair_offset = vec2::zero;
};

template <typename CharacterHandle>
inline navigate_path_result navigate_path(
	std::optional<ai_path_navigation_state>& navigation_opt,
	const vec2 bot_pos,
	const cosmos_navmesh& navmesh,
	CharacterHandle character,
	const real32 dt,
	const physics_path_hints* physics_hints = nullptr
) {
	navigate_path_result result;

	if (!navigation_opt.has_value()) {
		return result;
	}

	auto& navigation = *navigation_opt;

	/*
		If the path leads through a portal and the bot is now on a different
		island, the teleport has fired — this segment is complete.
	*/
	if (navigation.main.path.final_portal_exit.has_value()) {
		const auto current_island_opt = ::find_island_for_position(navmesh, bot_pos);

		if (current_island_opt.has_value()
			&& *current_island_opt != navigation.main.path.island_index
		) {
			result.path_completed = true;
			result.crosshair_offset = navigation.target_transform.get_direction() * 200.0f;
			navigation_opt.reset();
			return result;
		}
	}

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
		::advance_path_if_cell_reached(navigation, bot_pos, navmesh, cell_path_completed);
	}

	/*
		Check for exact destination mode: path is only completed when
		the bot reaches the exact target position, not just the target cell center.
	*/
	if (cell_path_completed) {
		if (navigation.exact_destination) {
			const auto target_pos = navigation.target_position();
			const float dist_to_exact = (bot_pos - target_pos).length();
			constexpr float EXACT_REACH_EPSILON = 30.0f;

			if (dist_to_exact > EXACT_REACH_EPSILON) {
				/*
					Not at exact position yet - continue navigating directly to target.
					
					NOTE: We delegate crosshair easing to get_navigation_movement_direction
					which handles it continuously from the final cell center to the exact destination.
				*/
				const auto dir_result = ::get_navigation_movement_direction(
					navigation,
					bot_pos,
					navmesh,
					dt
				);
				
				result.movement_direction = dir_result.movement_direction;
				result.crosshair_offset = dir_result.crosshair_offset;
				result.is_navigating = true;

				::debug_draw_path_navigation(navigation_opt, bot_pos, navmesh);
				return result;
			}
			else {
				/* Reached exact destination - mark as completed. */
				result.path_completed = true;
				result.crosshair_offset = navigation.target_transform.get_direction() * 200.0f;
				navigation_opt.reset();
				return result;
			}
		}
		else {
			/* Non-exact mode - path completed at cell center. */
			result.path_completed = true;
			navigation_opt.reset();
			return result;
		}
	}

	if (!is_inert) {
		::check_path_deviation(navigation, bot_pos, navmesh, physics_hints, nullptr);
	}

	/*
		Check if we're in the "post-cell-path" phase for exact destination mode.
		This happens when the cell path is complete but we haven't reached
		the exact destination yet.
	*/
	const auto current_target = ::get_current_path_target(navigation, navmesh);
	
	if (!current_target.has_value() && navigation.exact_destination) {
		/*
			Cell path is complete. Check if we've reached the exact destination.
		*/
		const auto target_pos = navigation.target_position();
		const float dist_to_exact = (bot_pos - target_pos).length();
		constexpr float EXACT_REACH_EPSILON = 30.0f;
		
		if (dist_to_exact <= EXACT_REACH_EPSILON) {
			/* Reached exact destination - mark as completed. */
			result.path_completed = true;
			result.crosshair_offset = navigation.target_transform.get_direction() * 200.0f;
			navigation_opt.reset();
			return result;
		}
		
		/* Not at exact position yet - continue navigating directly to target. */
	}

	/*
		Get movement direction from navigation state.
		Also calculates crosshair offset (un-normalized).
	*/
	const auto dir_result = ::get_navigation_movement_direction(
		navigation,
		bot_pos,
		navmesh,
		dt
	);

	result.movement_direction = dir_result.movement_direction;
	result.crosshair_offset = dir_result.crosshair_offset;

	/*
		Debug draw the path.
	*/
	::debug_draw_path_navigation(navigation_opt, bot_pos, navmesh);

	result.is_navigating = true;
	
	/*
		Calculate can_sprint: movement direction should be mostly parallel
		to the navigation direction (within ~15 degrees in the same direction).
		
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

			const auto nodes_n = navigation.main.path.nodes.size();
			const auto nodes_i = navigation.main.node_index;

			if (nodes_n <= 1 || nodes_i >= nodes_n - 1) {
				result.can_sprint = false;
				result.nearing_end = true;
			}
		}
	}
	
	return result;
}
