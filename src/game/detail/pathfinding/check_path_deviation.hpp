#pragma once
#include "game/detail/pathfinding.h"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/detail/pathfinding/path_helpers.hpp"

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
