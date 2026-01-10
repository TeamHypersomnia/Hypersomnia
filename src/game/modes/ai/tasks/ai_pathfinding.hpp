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

static constexpr std::size_t DEVIATION_CHECK_RANGE_V = 5;

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
		: pathfinding.main;

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
	bool& path_completed
) {
	path_completed = false;

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
				Check if we're in the half facing away from the previous cell.
			*/
			if (progress.node_index > 0) {
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
		if (main_path_finished && !pathfinding.main.path.final_portal_node.has_value()) {
			path_completed = true;
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

		if (nodes.empty() || current_idx >= nodes.size()) {
			return false;
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

		std::size_t reroute_target_idx = current_idx < nodes.size() ? current_idx : nodes.size() - 1;
		float reroute_closest_dist_sq = std::numeric_limits<float>::max();

		for (std::size_t i = start_check; i <= end_check; ++i) {
			const auto cell_world = ::cell_to_world(island, nodes[i].cell_xy);
			const auto dist_sq = (bot_pos - cell_world).length_sq();

			if (dist_sq < reroute_closest_dist_sq) {
				reroute_closest_dist_sq = dist_sq;
				reroute_target_idx = i;
			}
		}

		const auto reroute_target_world = ::cell_to_world(island, nodes[reroute_target_idx].cell_xy);
		auto rerouting_path = ::find_path_across_islands_many(navmesh, bot_pos, reroute_target_world, ctx);

		if (rerouting_path.has_value()) {
			pathfinding.rerouting = pathfinding_progress{
				std::move(*rerouting_path),
				0
			};
			pathfinding.main.node_index = reroute_target_idx;
		}
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
	const vec2 target_pos,
	const cosmos_navmesh& navmesh,
	pathfinding_context* ctx
) {
	/*
		Check if we're already pathfinding to this destination.
	*/
	const auto target_island_opt = ::find_island_for_position(navmesh, target_pos);

	if (!target_island_opt.has_value()) {
		return false;
	}

	const auto target_island = *target_island_opt;
	const auto& island = navmesh.islands[target_island];
	const auto target_cell = ::world_to_cell(island, target_pos);
	const auto new_target_cell_id = navmesh_cell_id{ target_island, target_cell };

	if (ai_state.is_pathfinding_active() &&
	    ai_state.pathfinding->target_cell_id == new_target_cell_id
	) {
		/*
			Already navigating to the same destination.
		*/
		return true;
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
		target_pos,
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
	const vec2 target_pos,
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
	const auto target_island_opt = ::find_island_for_position(navmesh, target_pos);

	if (!target_island_opt.has_value()) {
		return false;
	}

	const auto target_island = *target_island_opt;
	const auto& island = navmesh.islands[target_island];
	const auto target_cell = ::world_to_cell(island, target_pos);
	const auto new_target_cell_id = navmesh_cell_id{ target_island, target_cell };

	if (pathfinding_opt.has_value() &&
	    pathfinding_opt->target_cell_id == new_target_cell_id) {
		/*
			Already navigating to the same destination.
		*/
		return true;
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
		target_pos,
		new_target_cell_id
	};

	return true;
}

/*
	Calculate movement direction from pathfinding state.
	Also handles crosshair smoothing toward the next cell.
*/

inline std::optional<vec2> get_pathfinding_movement_direction(
	const ai_pathfinding_state& pathfinding,
	const vec2 bot_pos,
	const cosmos_navmesh& navmesh,
	vec2& target_crosshair_offset
) {
	const auto current_target_opt = ::get_current_path_target(pathfinding, navmesh);

	if (!current_target_opt.has_value()) {
		/*
			No valid current target, navigate directly to final target.
		*/
		const auto dir = pathfinding.target_position - bot_pos;
		target_crosshair_offset = dir;
		return vec2(dir).normalize();
	}

	const auto current_target = *current_target_opt;
	const auto dir = current_target - bot_pos;

	/*
		Calculate smoothed crosshair target by looking ahead on the path.
		When travelling from cell 0 to 1, with cell 2 ahead, the crosshair interpolates
		between looking at cell 1's center (when at cell 0) and cell 2's center (when at cell 1).
		
		The interpolated point lies on the imaginary line from current cell center to next cell center,
		with progress determined by bot's distance to the current cell center.
	*/
	vec2 look_ahead_target = current_target;

	const pathfinding_progress* active_progress_ptr = nullptr;

	if (pathfinding.rerouting.has_value()) {
		active_progress_ptr = &*pathfinding.rerouting;
	}
	else {
		active_progress_ptr = &pathfinding.main;
	}

	const auto& active_progress = *active_progress_ptr;
	const auto& path = active_progress.path;

	if (path.island_index < navmesh.islands.size()) {
		const auto& island = navmesh.islands[path.island_index];
		const auto cell_size = static_cast<float>(island.cell_size);

		if (cell_size > 0.0f && active_progress.node_index + 1 < path.nodes.size()) {
			const auto next_target = ::cell_to_world(island, path.nodes[active_progress.node_index + 1].cell_xy);

			/*
				Calculate progress as how close we are to the current cell center.
				At center of current cell (progress = 1.0): look fully at next cell.
				Far from current cell (progress = 0.0): look at current cell.
			*/
			const auto dist_to_current = (bot_pos - current_target).length();
			const auto t = std::clamp(1.0f - dist_to_current / cell_size, 0.0f, 1.0f);

			look_ahead_target = current_target + (next_target - current_target) * t;
		}
	}

	target_crosshair_offset = look_ahead_target - bot_pos;

	return vec2(dir).normalize();
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

		for (std::size_t i = 0; i + 1 < path.nodes.size(); ++i) {
			const auto from = ::cell_to_world(island, path.nodes[i].cell_xy);
			const auto to = ::cell_to_world(island, path.nodes[i + 1].cell_xy);

			if (rerouting) {
				DEBUG_LOGIC_STEP_LINES.emplace_back(i < progress.node_index ? orange : yellow, from, to);
			}
			else {
				DEBUG_LOGIC_STEP_LINES.emplace_back(i < progress.node_index ? red : green, from, to);
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
}
