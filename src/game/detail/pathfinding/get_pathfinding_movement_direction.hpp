#pragma once
#include "game/detail/pathfinding.h"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/detail/pathfinding/path_helpers.hpp"
#include "game/debug_drawing_settings.h"
#include "augs/math/arithmetical.h"
#include "game/common_state/cosmos_pathfinding.h"

/*
	Stuck rotation interval in seconds.
	When stuck on the same cell for this long, rotate crosshair by 90 degrees.
*/

static constexpr float STUCK_ROTATION_INTERVAL_SECS = 1.0f;

/*
	Result of get_pathfinding_movement_direction.
	Contains both movement direction and crosshair offset.
*/

struct pathfinding_direction_result {
	vec2 movement_direction = vec2::zero;
	vec2 crosshair_offset = vec2::zero;
};

/*
	Calculate movement direction from pathfinding state.
	Also handles crosshair smoothing toward the next cell.
	
	Returns struct containing:
	- movement_direction: normalized direction for movement
	- crosshair_offset: un-normalized crosshair offset
	- has_direction: true if there is a valid direction
	
	When on a rerouting path, correctly eases between the last node of the
	rerouting path and the target node of the main path for crosshair continuity.
	
	If stuck on the same cell for 2+ seconds, rotates the crosshair offset
	by 90 degrees to help the bot escape corners. The angle increases by
	90 degrees every 2 seconds.
	
	On the final tile segment (penultimate tile or exact destination approach),
	the bot eases towards the target transform's rotation. When t reaches 0.8,
	we snap to the exact target rotation because we will never reach t = 1.0
	exactly (due to epsilon-based path completion).
*/

inline pathfinding_direction_result get_pathfinding_movement_direction(
	ai_pathfinding_state& pathfinding,
	const vec2 bot_pos,
	const cosmos_navmesh& navmesh,
	const float dt
) {
	pathfinding_direction_result result;
	
	const auto current_target_opt = ::get_current_path_target(pathfinding, navmesh);
	const auto target_pos = pathfinding.target_position();
	const auto target_direction = pathfinding.target_transform.get_direction();

	if (!current_target_opt.has_value()) {
		/*
			No valid current target - cell path is complete.
			Navigate directly to final target with target transform rotation.
		*/
		const auto dir = target_pos - bot_pos;
		const auto& main_path = pathfinding.main.path;
		
		if (main_path.island_index < navmesh.islands.size() && !main_path.nodes.empty()) {
			const auto& island = navmesh.islands[main_path.island_index];
			const auto cell_size = static_cast<float>(island.cell_size);
			
			/* Use target direction directly since t >= 0.8 */
			result.crosshair_offset = target_direction * cell_size;
			result.movement_direction = vec2(dir).normalize();
			return result;
		}
		
		/* Default: just look in target direction */
		result.crosshair_offset = target_direction * 200.0f;
		result.movement_direction = vec2(dir).normalize();
		return result;
	}

	const auto current_target = *current_target_opt;
	const auto dir = current_target - bot_pos;

	/*
		Calculate smoothed crosshair target by looking ahead on the path.
		When travelling from cell 0 to 1, with cell 2 ahead, the crosshair interpolates
		between looking at cell 1's center (when at cell 0) and cell 2's center (when at cell 1).
		
		The interpolated point lies on the imaginary line from current cell center to next cell center,
		with progress determined by bot's distance to the current cell center.
		
		When on a rerouting path, at the last node of rerouting we ease towards the
		target node on the main path to preserve continuity.
		
		On the penultimate tile (no next cell), ease towards target transform's rotation.
		We finish rotation early (at t = 0.8) because epsilon-based path completion
		means we never reach t = 1.0 exactly.
	*/
	vec2 look_ahead_target = current_target;

	const bool is_rerouting = pathfinding.rerouting.has_value();

	const pathfinding_progress* active_progress_ptr = nullptr;

	if (is_rerouting) {
		active_progress_ptr = &*pathfinding.rerouting;
	}
	else {
		active_progress_ptr = &pathfinding.main;
	}

	const auto& active_progress = *active_progress_ptr;
	const auto& path = active_progress.path;
	bool crosshair_set = false;

	if (path.island_index < navmesh.islands.size()) {
		const auto& island = navmesh.islands[path.island_index];
		const auto cell_size = static_cast<float>(island.cell_size);

		if (cell_size > 0.0f) {
			vec2 next_target;
			bool has_next_target = false;
			bool ease_to_target_rotation = false;

			/*
				Calculate the effective cell distance for easing.
				For diagonal moves between cells, this is cell_size * sqrt(2).
			*/
			auto calc_cell_step_distance = [&](const vec2u from_cell, const vec2u to_cell) -> float {
				const int dx = static_cast<int>(to_cell.x) - static_cast<int>(from_cell.x);
				const int dy = static_cast<int>(to_cell.y) - static_cast<int>(from_cell.y);
				const bool is_diagonal = (dx != 0) && (dy != 0);
				return is_diagonal ? (cell_size * SQRT_2) : cell_size;
			};

			float effective_cell_distance = cell_size;

			if (active_progress.node_index + 1 < path.nodes.size()) {
				/*
					Normal case: look ahead to next cell on current path.
				*/
				const auto& curr_cell = path.nodes[active_progress.node_index].cell_xy;
				const auto& next_cell = path.nodes[active_progress.node_index + 1].cell_xy;
				next_target = ::cell_to_world(island, next_cell);
				
				/*
					For easing, use the distance from PREVIOUS cell to CURRENT cell,
					since that's the segment the bot is currently traveling on.
					This ensures smooth crosshair transition without jumps.
				*/
				if (active_progress.node_index > 0) {
					const auto& prev_cell = path.nodes[active_progress.node_index - 1].cell_xy;
					effective_cell_distance = calc_cell_step_distance(prev_cell, curr_cell);
				}
				else {
					/*
						At first node - use the distance to the next cell.
					*/
					effective_cell_distance = calc_cell_step_distance(curr_cell, next_cell);
				}
				has_next_target = true;
			}
			else if (is_rerouting) {
				/*
					At the last node of rerouting path - ease towards the target node on main path.
					This preserves continuity when transitioning from rerouting back to main path.
				*/
				const auto& main_path = pathfinding.main.path;
				const auto main_idx = pathfinding.main.node_index + 1;

				if (main_path.island_index < navmesh.islands.size() &&
				    main_idx < main_path.nodes.size()
				) {
					const auto& main_island = navmesh.islands[main_path.island_index];
					next_target = ::cell_to_world(main_island, main_path.nodes[main_idx].cell_xy);
					has_next_target = true;
				}
			}
			else {
				/*
					On penultimate tile (no next cell) - ease towards target rotation.
				*/
				ease_to_target_rotation = true;
			}

			if (has_next_target) {
				/*
					Calculate progress as how close we are to the current cell center.
					At center of current cell (progress = 1.0): look fully at next cell.
					Far from current cell (progress = 0.0): look at current cell.
					
					Use effective_cell_distance for proper interpolation on diagonal paths.
				*/
				const auto dist_to_current = (bot_pos - current_target).length();
				const auto t = std::clamp(1.0f - dist_to_current / effective_cell_distance, 0.0f, 1.0f);

				look_ahead_target = current_target + (next_target - current_target) * t;
			}
			else if (ease_to_target_rotation) {
				/*
					Ease towards the target transform's facing direction.
					
					The total ease distance depends on whether the last step was diagonal.
					We finish rotation at t = 0.8 because we will never reach t = 1.0 exactly.
				*/
				
				/*
					Calculate effective distance based on previous step (if any).
				*/
				float total_ease_distance = cell_size;
				
				if (active_progress.node_index > 0 && active_progress.node_index < path.nodes.size()) {
					const auto& prev_cell = path.nodes[active_progress.node_index - 1].cell_xy;
					const auto& curr_cell = path.nodes[active_progress.node_index].cell_xy;
					total_ease_distance = calc_cell_step_distance(prev_cell, curr_cell);
				}
				
				const auto dist_to_current = (bot_pos - current_target).length();
				
				/* Shrink time so we finish rotation before reaching the epsilon */
				const auto shrinked_time = 0.8f;
				const auto t = std::clamp((1.0f - dist_to_current / total_ease_distance) / shrinked_time, 0.0f, 1.0f);
				
				/* Look at current target direction */
				const auto look_at_dir = current_target - bot_pos;
				const auto look_at_length = look_at_dir.length();
				
				if (look_at_length > 0.0f) {
					/* Angular interpolation between looking at cell and target direction */
					const auto look_at_normalized = vec2(look_at_dir) / look_at_length;
					const auto target_normalized = vec2(target_direction).normalize();
					const auto interpolated_direction = augs::interp_angle(look_at_normalized, target_normalized, t);
					
					/* Use total_ease_distance for magnitude to match diagonal approach smoothly */
					result.crosshair_offset = interpolated_direction * total_ease_distance;
					
					if (DEBUG_DRAWING.draw_ai_info) {
						DEBUG_LOGIC_STEP_LINES.emplace_back(white, bot_pos, bot_pos + look_at_dir);
					}
				}
				else {
					result.crosshair_offset = target_direction * total_ease_distance;
				}
				
				crosshair_set = true;
			}
		}
	}

	if (!crosshair_set) {
		result.crosshair_offset = look_ahead_target - bot_pos;
	}

	/*
		Track stuck time on the same cell.
		If the bot is within the current target cell, track time.
		Rotate crosshair by 90 degrees every STUCK_ROTATION_INTERVAL_SECS.
	*/
	if (path.island_index < navmesh.islands.size() && active_progress.node_index < path.nodes.size()) {
		const auto current_cell_xy = path.nodes[active_progress.node_index].cell_xy;

		if (current_cell_xy == pathfinding.stuck_cell) {
			/*
				Still on the same cell - accumulate time.
			*/
			pathfinding.stuck_time += dt;

			if (pathfinding.stuck_time >= STUCK_ROTATION_INTERVAL_SECS) {
				/*
					Rotate the crosshair offset by 90 degrees for every 2-second interval.
				*/
				const int rotation_count = static_cast<int>(pathfinding.stuck_time / STUCK_ROTATION_INTERVAL_SECS);
				const float target_rotation = static_cast<float>(rotation_count) * (3.14159265f / 2.0f);

				const float ROTATION_SMOOTH_TIME = 0.5f;
				const float alpha = std::min(dt / ROTATION_SMOOTH_TIME, 1.0f);

				pathfinding.stuck_rotation += (target_rotation - pathfinding.stuck_rotation) * alpha;

				result.crosshair_offset = result.crosshair_offset.rotate_radians(pathfinding.stuck_rotation);
			}
		}
		else {
			/*
				Changed cells - reset stuck timer.
			*/
			pathfinding.stuck_rotation = 0.0f;
			pathfinding.stuck_cell = current_cell_xy;
			pathfinding.stuck_time = 0.0f;
		}
	}

	result.movement_direction = vec2(dir).normalize();
	return result;
}
