#pragma once
#include "game/detail/pathfinding.h"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/detail/pathfinding/path_helpers.hpp"

/*
	Advance along the current path when reaching cell centers.
	Does NOT clear pathfinding state when reaching portal - that should only happen
	when receiving a teleportation message.
	
	Cell advancement logic:
	- Advance when within epsilon of the next cell's center
	- OR when outside epsilon but in the half of the cell facing away from previous cell
	
	Diagonal cell advancement (separate logic):
	- When the next path node is diagonal relative to the previous, the bot will pass through
	  one of the two intermediate cardinal cells before reaching the diagonal cell.
	- For diagonal moves, we also advance if the bot is in EITHER of the two intermediate
	  cardinal cells (not on the path itself) and has moved past the center towards the diagonal.
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

		/*
			Separate diagonal navigation logic:
			When the current node is diagonal from the previous node, the bot will traverse
			through one of the two intermediate cardinal cells. Check if we're in one of those
			cells and have progressed sufficiently towards the diagonal destination.
		*/
		if (!should_advance && progress.node_index > 0) {
			const auto& prev_node = path.nodes[progress.node_index - 1];
			const auto prev_cell = prev_node.cell_xy;
			const auto curr_cell = node.cell_xy;
			
			/*
				Check if this is a diagonal move (both x and y changed).
			*/
			const int dx = static_cast<int>(curr_cell.x) - static_cast<int>(prev_cell.x);
			const int dy = static_cast<int>(curr_cell.y) - static_cast<int>(prev_cell.y);
			const bool is_diagonal_move = (dx != 0) && (dy != 0);
			
			if (is_diagonal_move) {
				/*
					Calculate the two intermediate cardinal cells.
					For a diagonal from (px, py) to (cx, cy):
					  - intermediate1: (cx, py) - same y as prev, same x as curr
					  - intermediate2: (px, cy) - same x as prev, same y as curr
				*/
				const auto intermediate1 = vec2u(curr_cell.x, prev_cell.y);
				const auto intermediate2 = vec2u(prev_cell.x, curr_cell.y);
				
				/*
					Check if bot is in one of the intermediate cells.
				*/
				const bool in_intermediate1 = ::is_within_cell(bot_pos, island, intermediate1);
				const bool in_intermediate2 = ::is_within_cell(bot_pos, island, intermediate2);
				
				if (in_intermediate1 || in_intermediate2) {
					/*
						We're in an intermediate cell. Check if we've progressed past the center
						towards the diagonal destination by checking both offset directions.
					*/
					const auto cell_center_intermediate = in_intermediate1 
						? ::cell_to_world(island, intermediate1) 
						: ::cell_to_world(island, intermediate2);
					const auto bot_offset_from_intermediate = bot_pos - cell_center_intermediate;
					
					/*
						For intermediate1 (cx, py): we need to move in the dy direction
						For intermediate2 (px, cy): we need to move in the dx direction
					*/
					bool progressed_enough = false;
					
					if (in_intermediate1) {
						/*
							In intermediate1 (cx, py). Need to check y-direction progress.
							dy > 0 means we're going down (higher y), so advance if bot_offset.y > 0
							dy < 0 means we're going up (lower y), so advance if bot_offset.y < 0
						*/
						progressed_enough = (dy > 0 && bot_offset_from_intermediate.y > 0) ||
						                    (dy < 0 && bot_offset_from_intermediate.y < 0);
					}
					else {
						/*
							In intermediate2 (px, cy). Need to check x-direction progress.
							dx > 0 means we're going right, so advance if bot_offset.x > 0
							dx < 0 means we're going left, so advance if bot_offset.x < 0
						*/
						progressed_enough = (dx > 0 && bot_offset_from_intermediate.x > 0) ||
						                    (dx < 0 && bot_offset_from_intermediate.x < 0);
					}
					
					if (progressed_enough) {
						should_advance = true;
					}
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
