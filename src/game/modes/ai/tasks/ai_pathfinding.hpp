#pragma once
#include "game/detail/pathfinding.h"
#include "game/modes/ai/ai_character_context.h"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/debug_drawing_settings.h"

/*
	Epsilon for determining when bot has reached a cell center.
*/

static constexpr float CELL_REACH_EPSILON = 15.0f;

/*
	Deviation check bounds: check nodes[index - DEVIATION_CHECK_RANGE_V .. index + DEVIATION_CHECK_RANGE_V].
*/

static constexpr std::size_t DEVIATION_CHECK_RANGE_V = 30;

/*
	Check if bot is within the given cell bounds.
*/

inline bool is_within_cell(
	const vec2 bot_pos,
	const cosmos_navmesh_island& island,
	const vec2u cell_xy
) {
	const auto cell_size = static_cast<float>(island.cell_size);
	const auto cell_world = ::cell_to_world(island, cell_xy);
	const auto half_size = cell_size / 2.0f;

	return 
		repro::fabs(bot_pos.x - cell_world.x) <= half_size &&
		repro::fabs(bot_pos.y - cell_world.y) <= half_size
	;
}

/*
	Get the world position of the current target cell in pathfinding.
*/

inline std::optional<vec2> get_current_path_target(
	const ai_pathfinding_state& pathfinding,
	const cosmos_navmesh& navmesh
) {
	/*
		If rerouting, use rerouting path.
	*/
	const auto& progress = pathfinding.rerouting.has_value() 
		? *pathfinding.rerouting 
		: pathfinding.main
	;

	const auto& path = progress.path;

	if (path.island_index >= navmesh.islands.size() ||
	    progress.node_index >= path.nodes.size()) {
		return std::nullopt;
	}

	const auto& island = navmesh.islands[path.island_index];
	const auto& node = path.nodes[progress.node_index];
	return ::cell_to_world(island, node.cell_xy);
}

/*
	Advance along the current path when reaching cell centers.
	Does NOT clear pathfinding state when reaching portal - that should only happen
	when receiving a teleportation message.
	
	Cell advancement logic:
	- Advance when within epsilon of the next cell's center
	- OR when outside epsilon but in the half of the cell facing away from previous cell
*/

inline void advance_path_if_cell_reached(
	ai_pathfinding_state& pathfinding,
	const vec2 bot_pos,
	const cosmos_navmesh& navmesh,
	bool& cell_path_completed
) {
	cell_path_completed = false;

	auto try_advance = [&](pathfinding_progress& progress) -> bool {
		const auto& path = progress.path;

		if (path.island_index >= navmesh.islands.size()) {
			return false;
		}

		const auto& island = navmesh.islands[path.island_index];

		if (progress.node_index >= path.nodes.size()) {
			return false;
		}

		const auto& node = path.nodes[progress.node_index];
		const auto cell_center = ::cell_to_world(island, node.cell_xy);
		const auto dist_to_center = (bot_pos - cell_center).length();

		bool should_advance = false;

		if (dist_to_center < CELL_REACH_EPSILON) {
			/*
				Within epsilon of cell center - advance.
			*/
			should_advance = true;
		}
		else if (::is_within_cell(bot_pos, island, node.cell_xy)) {
			/*
				We're in the cell but outside epsilon.
			*/
			if (progress.node_index > 0) {
				/*
					Check if we're in the half facing away from the previous cell.
				*/
				const auto& prev_node = path.nodes[progress.node_index - 1];
				const auto prev_center = ::cell_to_world(island, prev_node.cell_xy);
				const auto to_prev = prev_center - cell_center;
				const auto bot_offset = bot_pos - cell_center;

				/*
					If dot product is negative, we're on the side opposite to prev_center.
				*/
				if (to_prev.dot(bot_offset) < 0.0f) {
					should_advance = true;
				}
			}
			else if (progress.node_index + 1 < path.nodes.size()) {
				/*
					node_index == 0: Check if we're in the half facing towards the next cell.
				*/
				const auto& next_node = path.nodes[progress.node_index + 1];
				const auto next_center = ::cell_to_world(island, next_node.cell_xy);
				const auto to_next = next_center - cell_center;
				const auto bot_offset = bot_pos - cell_center;

				/*
					If dot product is positive, we're on the side towards next_center.
				*/
				if (to_next.dot(bot_offset) > 0.0f) {
					should_advance = true;
				}
			}
		}

		if (should_advance) {
			++progress.node_index;

			/*
				Check if we've finished this path segment.
				Don't clear pathfinding - let teleportation message handle that.
			*/
			if (progress.node_index >= path.nodes.size()) {
				return true;
			}
		}

		return false;
	};

	bool main_path_finished = false;

	if (pathfinding.rerouting.has_value()) {
		const bool finished = try_advance(*pathfinding.rerouting);

		/*
			If we've finished rerouting, we're back on the main path.
		*/
		if (finished || 
		    pathfinding.rerouting->node_index >= pathfinding.rerouting->path.nodes.size()
		) {
			pathfinding.clear_rerouting();
		}
	}
	else {
		main_path_finished = try_advance(pathfinding.main);

		/*
			If we've finished main path and there's no portal,
			signal path completion (destination reached).
		*/
		if (main_path_finished) {
			cell_path_completed = true;
		}
		/*
			If path has portal, don't clear - wait for teleportation message.
		*/
	}
}

/*
	Check if bot has fallen off the path and needs rerouting.
	
	The logic is:
	1. BEFORE looking for the closest tile, check O(1) if we're on current or previous node_index.
	2. ONLY if we're outside those two cells, look for the closest one in ±DEVIATION_CHECK_RANGE_V range.
	3. If found within bounds, update node_index (unless it's current or current-1).
	4. If not found, initialize rerouting path to the closest cell.
	
	This applies the same way to both the rerouting and main paths - when out of bounds
	on either path, recalculate the rerouting path.
*/

inline void check_path_deviation(
	ai_pathfinding_state& pathfinding,
	const vec2 bot_pos,
	const cosmos_navmesh& navmesh,
	pathfinding_context* ctx
) {
	/*
		Lambda to check if we've deviated from a path and need rerouting.
		Returns true if we're still on the path and can continue.
		Returns false if we need to recalculate rerouting.
	*/
	auto check_path_out_of_bounds = [&](
		pathfinding_progress& progress,
		const cosmos_navmesh_island& island
	) -> bool {
		const auto& nodes = progress.path.nodes;
		const auto current_idx = progress.node_index;

		if (nodes.empty()) {
			return false;
		}

		/*
			Path is completed - we're past all nodes.
			This is not a deviation, so return true.
		*/
		if (current_idx >= nodes.size()) {
			return true;
		}

		/*
			O(1) check: are we on the current or previous cell?
		*/
		if (::is_within_cell(bot_pos, island, nodes[current_idx].cell_xy)) {
			return true;
		}

		if (current_idx > 0 && ::is_within_cell(bot_pos, island, nodes[current_idx - 1].cell_xy)) {
			return true;
		}

		/*
			We're not on current or previous cell - search nearby cells.
		*/
		const auto start_check = current_idx >= DEVIATION_CHECK_RANGE_V ? current_idx - DEVIATION_CHECK_RANGE_V : 0;
		const auto end_check = std::min(current_idx + DEVIATION_CHECK_RANGE_V, static_cast<std::size_t>(nodes.size()) - 1);

		const auto found_cell_idx = [&]() -> std::optional<uint32_t> {
			for (std::size_t i = start_check; i <= end_check; ++i) {
				if (::is_within_cell(bot_pos, island, nodes[i].cell_xy)) {
					return i;
				}
			}

			return std::nullopt;
		}();

		if (found_cell_idx.has_value()) {
			/*
				We're still on the path at a different cell.
				Only update if NOT on current or current-1 (already checked above).
			*/
			const auto found_idx = *found_cell_idx;
			const bool on_current = (found_idx == current_idx);
			const bool on_previous = (current_idx > 0 && found_idx == current_idx - 1);

			if (!on_current && !on_previous) {
				progress.node_index = found_idx;
			}

			return true;
		}

		/*
			Not on any nearby cell - we've deviated.
		*/
		return false;
	};

	/*
		Calculate rerouting target cell.
		
		Only allows rerouting to:
		- Unoccupied cells (value == 0)
		- The target portal cell (if the main path ends at a portal)
		
		After calculating the rerouting path, trims it if any of its nodes
		coincide with unoccupied cells on the main path within deviation range.
	*/
	auto calculate_rerouting = [&](const pathfinding_progress& main_progress) {
		const auto& nodes = main_progress.path.nodes;
		const auto current_idx = main_progress.node_index;

		if (main_progress.path.island_index >= navmesh.islands.size() || nodes.empty()) {
			return;
		}

		const auto& island = navmesh.islands[main_progress.path.island_index];

		const auto start_check = current_idx >= DEVIATION_CHECK_RANGE_V ? current_idx - DEVIATION_CHECK_RANGE_V : 0;
		const auto end_check = std::min(current_idx + DEVIATION_CHECK_RANGE_V, static_cast<std::size_t>(nodes.size()) - 1);

		/*
			Find the closest valid rerouting target cell within deviation range.
			Only allow unoccupied cells OR the target portal cell.
		*/
		std::optional<std::size_t> reroute_target_idx;
		float reroute_closest_dist_sq = std::numeric_limits<float>::max();

		for (std::size_t i = start_check; i <= end_check; ++i) {
			const auto& cell_xy = nodes[i].cell_xy;

			/*
				Only allow unoccupied cells.
			*/

			const bool is_valid_target = island.is_cell_unoccupied(cell_xy);

			if (!is_valid_target) {
				continue;
			}

			const auto cell_world = ::cell_to_world(island, cell_xy);
			const auto dist_sq = (bot_pos - cell_world).length_sq();

			if (dist_sq < reroute_closest_dist_sq) {
				reroute_closest_dist_sq = dist_sq;
				reroute_target_idx = i;
			}
		}

		if (!reroute_target_idx.has_value()) {
			/*
				No valid rerouting target found.
			*/
			return;
		}

		const auto reroute_target_world = ::cell_to_world(island, nodes[*reroute_target_idx].cell_xy);
		auto rerouting_path = ::find_path_across_islands_many(navmesh, bot_pos, reroute_target_world, ctx);

		if (!rerouting_path.has_value()) {
			return;
		}

		/*
			Trim the rerouting path if any of its nodes coincide with unoccupied cells
			on the main path within deviation range. Keep the furthest matching node.
		*/
		std::size_t trim_to_idx = rerouting_path->nodes.size();
		std::size_t best_main_idx = *reroute_target_idx;

		for (std::size_t reroute_i = 0; reroute_i < rerouting_path->nodes.size(); ++reroute_i) {
			const auto& reroute_cell = rerouting_path->nodes[reroute_i].cell_xy;

			for (std::size_t main_i = start_check; main_i <= end_check; ++main_i) {
				if (main_i >= nodes.size()) {
					break;
				}

				const auto& main_cell = nodes[main_i].cell_xy;
				const auto main_cell_value = island.get_cell(main_cell);

				/*
					Only match unoccupied cells on the main path.
				*/
				if (!::is_cell_unoccupied(main_cell_value)) {
					continue;
				}

				if (reroute_cell == main_cell) {
					/*
						Found a match. If this main path index is further in progress,
						trim the rerouting path here.
					*/
					if (main_i > best_main_idx) {
						best_main_idx = main_i;
						trim_to_idx = reroute_i + 1;
					}
					else if (main_i == best_main_idx && reroute_i + 1 < trim_to_idx) {
						trim_to_idx = reroute_i + 1;
					}
				}
			}
		}

		if (trim_to_idx < rerouting_path->nodes.size()) {
			rerouting_path->nodes.resize(trim_to_idx);
		}

		if (rerouting_path->nodes.empty()) {
			/*
				Rerouting path trimmed to nothing - we're already on the main path.
			*/
			pathfinding.main.node_index = best_main_idx;
			return;
		}

		pathfinding.rerouting = pathfinding_progress{
			std::move(*rerouting_path),
			0
		};
		pathfinding.main.node_index = best_main_idx;
	};

	const auto& main_path = pathfinding.main.path;

	if (main_path.island_index >= navmesh.islands.size()) {
		return;
	}

	const auto& main_island = navmesh.islands[main_path.island_index];

	/*
		Check if rerouting path is active.
	*/
	if (pathfinding.rerouting.has_value()) {
		auto& rerouting = *pathfinding.rerouting;

		if (rerouting.path.island_index >= navmesh.islands.size()) {
			/*
				Invalid rerouting path - recalculate.
			*/
			calculate_rerouting(pathfinding.main);
			return;
		}

		const auto& rerouting_island = navmesh.islands[rerouting.path.island_index];

		if (!check_path_out_of_bounds(rerouting, rerouting_island)) {
			/*
				Fell off rerouting path - recalculate rerouting to main path.
			*/
			calculate_rerouting(pathfinding.main);
		}

		return;
	}

	/*
		Check main path.
	*/
	if (!check_path_out_of_bounds(pathfinding.main, main_island)) {
		/*
			Fell off main path - begin rerouting.
		*/
		calculate_rerouting(pathfinding.main);
	}
}

/*
	Check if the bot is currently standing on a portal cell.
*/

inline bool is_on_portal_cell(
	const vec2 bot_pos,
	const cosmos_navmesh& navmesh
) {
	const auto island_opt = ::find_island_for_position(navmesh, bot_pos);

	if (!island_opt.has_value()) {
		return false;
	}

	const auto& island = navmesh.islands[*island_opt];
	const auto cell = ::world_to_cell(island, bot_pos);
	const auto cell_value = island.get_cell(cell);

	return ::is_cell_portal(cell_value);
}

/*
	Start pathfinding to a target position.
	Updates the ai_state's optional pathfinding field.
	Returns true if pathfinding was initiated.
	
	Note: Does not initiate pathfinding if bot is standing on a portal cell
	(to allow portal teleportation to complete first).
*/

inline bool start_pathfinding_to(
	arena_mode_ai_state& ai_state,
	const vec2 bot_pos,
	const transformr target_transform,
	const cosmos_navmesh& navmesh,
	pathfinding_context* ctx
) {
	/*
		Check if we're already pathfinding to this destination.
	*/
	const auto target_pos = target_transform.pos;
	const auto target_island_opt = ::find_island_for_position(navmesh, target_pos);

	if (!target_island_opt.has_value()) {
		return false;
	}

	const auto target_island = *target_island_opt;
	const auto& island = navmesh.islands[target_island];
	const auto target_cell = ::world_to_cell(island, target_pos);
	const auto new_target_cell_id = cell_on_navmesh(target_island, target_cell);

	if (ai_state.is_pathfinding_active() &&
	    ai_state.pathfinding->target_cell_id == new_target_cell_id
	) {
		/*
			Already navigating to the same destination.
		*/
		return true;
	}

	/*
		Check if we're already at the target cell.
		If so, don't start pathfinding - we've already arrived.
		This prevents restarting pathfinding after path completed.
	*/
	if (::is_within_cell(bot_pos, island, target_cell)) {
		return false;
	}

	/*
		Calculate path to next portal or directly to destination.
		Using find_path_across_islands_many which only returns path to the next portal.
	*/
	auto path = ::find_path_across_islands_many(navmesh, bot_pos, target_pos, ctx);

	if (!path.has_value()) {
		return false;
	}

	ai_state.pathfinding = ai_pathfinding_state {
		pathfinding_progress{ std::move(*path), 0 },
		std::nullopt,
		target_transform,
		new_target_cell_id
	};

	return true;
}

/*
	Overload for std::optional<ai_pathfinding_state> - useful for test_mode.
*/

inline bool start_pathfinding_to(
	std::optional<ai_pathfinding_state>& pathfinding_opt,
	const vec2 bot_pos,
	const transformr target_transform,
	const cosmos_navmesh& navmesh,
	pathfinding_context* ctx
) {
	/*
		Don't initiate new pathfinding while standing on a portal cell.
		Existing pathfinding sessions can continue, but new ones should wait
		until the bot has teleported through the portal.
	*/
	if (!pathfinding_opt.has_value() && ::is_on_portal_cell(bot_pos, navmesh)) {
		return false;
	}

	/*
		Check if we're already pathfinding to this destination.
	*/
	const auto target_pos = target_transform.pos;
	const auto target_island_opt = ::find_island_for_position(navmesh, target_pos);

	if (!target_island_opt.has_value()) {
		return false;
	}

	const auto target_island = *target_island_opt;
	const auto& island = navmesh.islands[target_island];
	const auto target_cell = ::world_to_cell(island, target_pos);
	const auto new_target_cell_id = cell_on_navmesh(target_island, target_cell);

	if (pathfinding_opt.has_value() &&
	    pathfinding_opt->target_cell_id == new_target_cell_id) {
		/*
			Already navigating to the same destination.
		*/
		return true;
	}

	/*
		Check if we're already at the target cell.
		If so, don't start pathfinding - we've already arrived.
		This prevents restarting pathfinding after path completed.
	*/
	if (::is_within_cell(bot_pos, island, target_cell)) {
		return false;
	}

	/*
		Calculate path to next portal or directly to destination.
		Using find_path_across_islands_many which only returns path to the next portal.
	*/
	auto path = ::find_path_across_islands_many(navmesh, bot_pos, target_pos, ctx);

	if (!path.has_value()) {
		return false;
	}

	pathfinding_opt = ai_pathfinding_state{
		pathfinding_progress{ std::move(*path), 0 },
		std::nullopt,
		target_transform,
		new_target_cell_id
	};

	return true;
}

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
	bool has_direction = false;
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
	
	/*
		Shared lambda for calculating eased crosshair offset.
		
		t interpolates between:
		- t = 0: looking at look_at_point
		- t = 1: looking in target transform's direction
		
		Uses augs::interp_angle for angular (rotational) interpolation.
		This avoids abrupt rotation changes when the bot passes through the look-at point.
		
		Parameters:
		- look_at_point: The point to look at when t = 0
		- remaining_distance: How far we still have to travel
		- total_ease_distance: The total easing distance (from t=0 point to t=1 point)
	*/
	const auto calc_eased_crosshair = [&](const vec2 look_at_point, const float remaining_distance, const float total_ease_distance) -> vec2 {
		const auto look_at_dir = look_at_point - bot_pos;
		const auto look_at_length = look_at_dir.length();
		
		if (look_at_length <= 0.0f || total_ease_distance <= 0.0f) {
			return target_direction * 200.0f;
		}
		
		const auto t = std::clamp(1.0f - remaining_distance / total_ease_distance, 0.0f, 1.0f);
		
		/* Normalize both directions for angular interpolation */
		const auto look_at_normalized = vec2(look_at_dir) / look_at_length;
		const auto target_normalized = vec2(target_direction).normalize();
		
		/* Angular interpolation between the two directions */
		const auto interpolated_direction = augs::interp_angle(look_at_normalized, target_normalized, t);
		
		/* Return the interpolated direction scaled by the look-at length */
		return interpolated_direction * look_at_length;
	};

	if (!current_target_opt.has_value()) {
		/*
			No valid current target - cell path is complete.
			Navigate directly to final target.
			
			For exact destination mode, continue easing the crosshair based on
			distance to exact destination. This continues the easing that started
			on the penultimate tile, with the same t calculation.
		*/
		const auto dir = target_pos - bot_pos;
		const auto dist_to_exact = dir.length();
		
		if (pathfinding.exact_destination && dist_to_exact > 0.0f) {
			/*
				Get the last cell info (if available) to calculate proper easing.
				The t value must be continuous with what we calculated on the penultimate tile.
			*/
			const auto& main_path = pathfinding.main.path;
			
			if (main_path.island_index < navmesh.islands.size() && !main_path.nodes.empty()) {
				const auto& island = navmesh.islands[main_path.island_index];
				const auto cell_size = static_cast<float>(island.cell_size);
				const auto last_cell = main_path.nodes.back().cell_xy;
				const auto last_cell_center = ::cell_to_world(island, last_cell);
				
				/*
					Total ease distance = cell_size + dist(cell_center, exact_destination)
					Remaining distance = dist(bot_pos, exact_destination)
					
					At transition (cell center):
					- Penultimate branch had: remaining = 0 + dist_cell_to_exact
					- Post-cell-path branch has: remaining = dist_to_exact = dist_cell_to_exact
					These are the same, so t is continuous!
				*/
				const auto dist_cell_to_exact = (target_pos - last_cell_center).length();
				const auto total_ease_distance = cell_size + dist_cell_to_exact;
				
				result.crosshair_offset = calc_eased_crosshair(target_pos, dist_to_exact, total_ease_distance);
				result.movement_direction = vec2(dir).normalize();
				result.has_direction = true;
				return result;
			}
		}
		
		/* Default: just look in target direction */
		result.crosshair_offset = target_direction * 200.0f;
		result.movement_direction = vec2(dir).normalize();
		result.has_direction = true;
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
		
		On the penultimate tile (no next cell), ease towards the target transform's rotation.
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

			if (active_progress.node_index + 1 < path.nodes.size()) {
				/*
					Normal case: look ahead to next cell on current path.
				*/
				next_target = ::cell_to_world(island, path.nodes[active_progress.node_index + 1].cell_xy);
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
					This helps the bot face the correct direction when reaching a waypoint.
				*/
				ease_to_target_rotation = true;
			}

			if (has_next_target) {
				/*
					Calculate progress as how close we are to the current cell center.
					At center of current cell (progress = 1.0): look fully at next cell.
					Far from current cell (progress = 0.0): look at current cell.
				*/
				const auto dist_to_current = (bot_pos - current_target).length();
				const auto t = std::clamp(1.0f - dist_to_current / cell_size, 0.0f, 1.0f);

				look_ahead_target = current_target + (next_target - current_target) * t;
			}
			else if (ease_to_target_rotation) {
				/*
					Ease towards the target transform's facing direction.
					
					Total ease distance: cell_size + distance from cell center to exact destination.
					Remaining distance: distance from bot to cell center + distance from cell center to exact.
					
					At transition to post-cell-path branch:
					- dist_to_current = 0 (at cell center)
					- remaining = 0 + dist_cell_to_exact = dist_cell_to_exact
					- This matches the post-cell-path calculation where remaining = dist_to_exact
					
					So t is continuous across both branches!
				*/
				const auto dist_to_current = (bot_pos - current_target).length();
				const auto dist_cell_to_exact = (target_pos - current_target).length();
				const auto total_ease_distance = cell_size + dist_cell_to_exact;
				const auto remaining_distance = dist_to_current + dist_cell_to_exact;
				
				result.crosshair_offset = calc_eased_crosshair(current_target, remaining_distance, total_ease_distance);
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
	result.has_direction = true;
	return result;
}

/*
	Debug draw the current pathfinding state.
*/

inline void debug_draw_pathfinding(
	const std::optional<ai_pathfinding_state>& pathfinding_opt,
	const vec2 bot_pos,
	const cosmos_navmesh& navmesh
) {
	if (!DEBUG_DRAWING.draw_ai_info || !pathfinding_opt.has_value()) {
		return;
	}

	const auto& pathfinding = *pathfinding_opt;

	auto draw_path = [&](const pathfinding_progress& progress, bool rerouting) {
		const auto& path = progress.path;

		if (path.island_index >= navmesh.islands.size()) {
			return;
		}

		const auto& island = navmesh.islands[path.island_index];
		const auto cell_size = static_cast<float>(island.cell_size);

		for (std::size_t i = 0; i < path.nodes.size(); ++i) {
			const auto& node = path.nodes[i];
			const auto cell_center = ::cell_to_world(island, node.cell_xy);

			/*
				Draw cell AABB with smaller alpha.
			*/
			rgba cell_color;
			if (rerouting) {
				cell_color = i < progress.node_index ? rgba(255, 0, 0, 40) : rgba(255, 255, 0, 40);
			}
			else {
				cell_color = i < progress.node_index ? rgba(255, 0, 0, 40) : rgba(0, 255, 0, 40);
			}

			const auto cell_size_vec = vec2(cell_size, cell_size);
			DEBUG_LOGIC_STEP_RECTS.emplace_back(cell_color, cell_center, cell_size_vec, 0.0f);

			/*
				Draw line to next cell.
			*/
			if (i + 1 < path.nodes.size()) {
				const auto to = ::cell_to_world(island, path.nodes[i + 1].cell_xy);

				if (rerouting) {
					DEBUG_LOGIC_STEP_LINES.emplace_back(i < progress.node_index ? red : yellow, cell_center, to);
				}
				else {
					DEBUG_LOGIC_STEP_LINES.emplace_back(i < progress.node_index ? red : green, cell_center, to);
				}
			}
		}
	};

	draw_path(pathfinding.main, false);

	if (pathfinding.rerouting.has_value()) {
		draw_path(*pathfinding.rerouting, true);
	}

	if (const auto target = ::get_current_path_target(pathfinding, navmesh)) {
		DEBUG_LOGIC_STEP_LINES.emplace_back(cyan, bot_pos, *target);
	}

	if (pathfinding.exact_destination) {
		DEBUG_LOGIC_STEP_LINES.emplace_back(cyan, bot_pos, pathfinding.target_position());
	}
}
