#pragma once
#include "game/detail/pathfinding.h"
#include "game/modes/ai/ai_character_context.h"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/debug_drawing_settings.h"
#include "game/inferred_caches/physics_world_cache.h"

/*
	Epsilon for determining when bot has reached a cell center.
*/

static constexpr float CELL_REACH_EPSILON = 15.0f;
static constexpr float FAT_LOS_WIDTH = 100.0f;

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
	if (!pathfinding.is_active || pathfinding.paths.empty()) {
		return std::nullopt;
	}

	/*
		If rerouting, use rerouting path.
	*/
	if (pathfinding.rerouting_paths.has_value()) {
		const auto& rerouting = *pathfinding.rerouting_paths;

		if (pathfinding.rerouting_path_index < rerouting.size()) {
			const auto& path = rerouting[pathfinding.rerouting_path_index];

			if (path.island_index < navmesh.islands.size() && 
			    pathfinding.rerouting_node_index < path.nodes.size()) {
				const auto& island = navmesh.islands[path.island_index];
				const auto& node = path.nodes[pathfinding.rerouting_node_index];
				return ::cell_to_world(island, node.cell_xy);
			}
		}

		return std::nullopt;
	}

	/*
		Use main path.
	*/
	if (pathfinding.current_path_index < pathfinding.paths.size()) {
		const auto& path = pathfinding.paths[pathfinding.current_path_index];

		if (path.island_index < navmesh.islands.size() && 
		    pathfinding.current_node_index < path.nodes.size()) {
			const auto& island = navmesh.islands[path.island_index];
			const auto& node = path.nodes[pathfinding.current_node_index];
			return ::cell_to_world(island, node.cell_xy);
		}
	}

	return std::nullopt;
}

/*
	Advance along the current path when reaching cell centers.
*/

inline void advance_path_if_reached(
	ai_pathfinding_state& pathfinding,
	const vec2 bot_pos,
	const cosmos_navmesh& navmesh
) {
	if (!pathfinding.is_active) {
		return;
	}

	auto try_advance = [&](
		std::size_t& path_idx,
		std::size_t& node_idx,
		const std::vector<pathfinding_path>& paths
	) {
		if (path_idx >= paths.size()) {
			return;
		}

		const auto& path = paths[path_idx];

		if (path.island_index >= navmesh.islands.size()) {
			return;
		}

		const auto& island = navmesh.islands[path.island_index];

		if (node_idx >= path.nodes.size()) {
			return;
		}

		const auto& node = path.nodes[node_idx];
		const auto cell_center = ::cell_to_world(island, node.cell_xy);
		const auto dist_to_center = (bot_pos - cell_center).length();

		if (dist_to_center < CELL_REACH_EPSILON) {
			++node_idx;

			/*
				If we've finished this path segment, move to next path.
			*/
			if (node_idx >= path.nodes.size()) {
				if (path.final_portal_node.has_value()) {
					/*
						Teleport through portal - move to next path segment.
					*/
					++path_idx;
					node_idx = 0;
				}
			}
		}
	};

	if (pathfinding.rerouting_paths.has_value()) {
		auto& rerouting = *pathfinding.rerouting_paths;
		try_advance(pathfinding.rerouting_path_index, pathfinding.rerouting_node_index, rerouting);

		/*
			If we've finished rerouting, we're back on the main path.
		*/
		if (pathfinding.rerouting_path_index >= rerouting.size()) {
			pathfinding.rerouting_paths.reset();
			pathfinding.rerouting_path_index = 0;
			pathfinding.rerouting_node_index = 0;
		}
	}
	else {
		try_advance(pathfinding.current_path_index, pathfinding.current_node_index, pathfinding.paths);

		/*
			If we've finished all paths, clear pathfinding state.
		*/
		if (pathfinding.current_path_index >= pathfinding.paths.size()) {
			pathfinding.clear();
		}
	}
}

/*
	Check if bot has fallen off the path and needs rerouting.
*/

inline void check_path_deviation(
	ai_pathfinding_state& pathfinding,
	const vec2 bot_pos,
	const cosmos_navmesh& navmesh,
	pathfinding_context* ctx
) {
	if (!pathfinding.is_active || pathfinding.paths.empty()) {
		return;
	}

	/*
		If already rerouting, check if we fell off the rerouting path.
	*/
	if (pathfinding.rerouting_paths.has_value()) {
		/*
			Recalculate rerouting to the main path cell.
		*/
		const auto& main_path = pathfinding.paths[pathfinding.current_path_index];

		if (main_path.island_index >= navmesh.islands.size() ||
		    pathfinding.current_node_index >= main_path.nodes.size()) {
			pathfinding.clear();
			return;
		}

		const auto& target_island = navmesh.islands[main_path.island_index];
		const auto target_cell = main_path.nodes[pathfinding.current_node_index].cell_xy;
		const auto target_world = ::cell_to_world(target_island, target_cell);

		auto new_rerouting = ::find_path_across_islands_many_full(navmesh, bot_pos, target_world, ctx);

		if (!new_rerouting.empty()) {
			pathfinding.rerouting_paths = std::move(new_rerouting);
			pathfinding.rerouting_path_index = 0;
			pathfinding.rerouting_node_index = 0;
		}

		return;
	}

	/*
		Check if we're on any of the nearby path cells.
	*/
	const auto& current_path = pathfinding.paths[pathfinding.current_path_index];

	if (current_path.island_index >= navmesh.islands.size()) {
		pathfinding.clear();
		return;
	}

	const auto& island = navmesh.islands[current_path.island_index];
	const auto& nodes = current_path.nodes;
	const auto current_idx = pathfinding.current_node_index;

	/*
		Check nodes[index - 5 .. index + 5] to find the closest one.
	*/
	const auto start_check = current_idx >= 5 ? current_idx - 5 : 0;
	const auto end_check = std::min(current_idx + 5, nodes.size() - 1);

	std::optional<std::size_t> closest_within_bounds;
	float closest_dist_sq = std::numeric_limits<float>::max();

	for (std::size_t i = start_check; i <= end_check; ++i) {
		const auto cell_world = ::cell_to_world(island, nodes[i].cell_xy);
		const auto dist_sq = (bot_pos - cell_world).length_sq();

		if (dist_sq < closest_dist_sq) {
			closest_dist_sq = dist_sq;

			if (::is_within_cell(bot_pos, island, nodes[i].cell_xy)) {
				closest_within_bounds = i;
			}
		}
	}

	if (closest_within_bounds.has_value()) {
		/*
			We're still on the path, update index if necessary.
		*/
		pathfinding.current_node_index = *closest_within_bounds;
		return;
	}

	/*
		We've fallen off - begin rerouting to the closest cell.
	*/
	std::size_t reroute_target_idx = current_idx;
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
	auto rerouting_path = ::find_path_across_islands_many_full(navmesh, bot_pos, reroute_target_world, ctx);

	if (!rerouting_path.empty()) {
		pathfinding.rerouting_paths = std::move(rerouting_path);
		pathfinding.rerouting_path_index = 0;
		pathfinding.rerouting_node_index = 0;
		pathfinding.current_node_index = reroute_target_idx;
	}
}

/*
	Start pathfinding to a target position.
	Returns true if pathfinding was initiated or FLoS exists.
*/

inline bool start_pathfinding_to(
	ai_pathfinding_state& pathfinding,
	const vec2 bot_pos,
	const vec2 target_pos,
	const cosmos_navmesh& navmesh,
	const physics_world_cache& physics,
	const si_scaling& si,
	const entity_id bot_entity,
	pathfinding_context* ctx
) {
	/*
		Check for direct FLoS first.
	*/
	if (::fat_line_of_sight(physics, si, bot_pos, target_pos, FAT_LOS_WIDTH, bot_entity)) {
		pathfinding.clear();
		pathfinding.target_position = target_pos;
		return true;
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

	if (pathfinding.is_active &&
	    pathfinding.target_island == target_island &&
	    pathfinding.target_cell == target_cell) {
		/*
			Already navigating to the same destination.
		*/
		return true;
	}

	/*
		Calculate new path.
	*/
	auto paths = ::find_path_across_islands_many_full(navmesh, bot_pos, target_pos, ctx);

	if (paths.empty()) {
		return false;
	}

	pathfinding.clear();
	pathfinding.paths = std::move(paths);
	pathfinding.current_path_index = 0;
	pathfinding.current_node_index = 0;
	pathfinding.target_position = target_pos;
	pathfinding.target_island = target_island;
	pathfinding.target_cell = target_cell;
	pathfinding.is_active = true;

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
	if (!pathfinding.is_active) {
		return std::nullopt;
	}

	/*
		If we have direct FLoS (no paths but active), navigate directly.
	*/
	if (pathfinding.paths.empty()) {
		const auto dir = pathfinding.target_position - bot_pos;
		target_crosshair_offset = dir;
		return dir.normalize();
	}

	const auto current_target_opt = ::get_current_path_target(pathfinding, navmesh);

	if (!current_target_opt.has_value()) {
		return std::nullopt;
	}

	const auto current_target = *current_target_opt;
	const auto dir = current_target - bot_pos;

	/*
		Calculate smoothed crosshair target by looking ahead on the path.
		This creates smooth turning at corners.
	*/
	vec2 look_ahead_target = current_target;

	const auto* active_paths = pathfinding.rerouting_paths.has_value() 
		? &(*pathfinding.rerouting_paths) 
		: &pathfinding.paths;
	const auto active_path_idx = pathfinding.rerouting_paths.has_value()
		? pathfinding.rerouting_path_index
		: pathfinding.current_path_index;
	const auto active_node_idx = pathfinding.rerouting_paths.has_value()
		? pathfinding.rerouting_node_index
		: pathfinding.current_node_index;

	if (active_path_idx < active_paths->size()) {
		const auto& path = (*active_paths)[active_path_idx];

		if (path.island_index < navmesh.islands.size()) {
			const auto& island = navmesh.islands[path.island_index];

			if (active_node_idx + 1 < path.nodes.size()) {
				const auto next_target = ::cell_to_world(island, path.nodes[active_node_idx + 1].cell_xy);

				/*
					Interpolate between current and next target based on distance.
				*/
				const auto dist_to_current = (bot_pos - current_target).length();
				const auto cell_size = static_cast<float>(island.cell_size);

				if (cell_size > 0.0f) {
					const auto t = std::clamp(1.0f - dist_to_current / cell_size, 0.0f, 1.0f);
					look_ahead_target = current_target + (next_target - current_target) * t;
				}
			}
		}
	}

	target_crosshair_offset = look_ahead_target - bot_pos;

	return dir.normalize();
}

/*
	Debug draw the current pathfinding state.
*/

inline void debug_draw_pathfinding(
	const ai_pathfinding_state& pathfinding,
	const vec2 bot_pos,
	const cosmos_navmesh& navmesh
) {
	if (!DEBUG_DRAWING.draw_ai_info || !pathfinding.is_active) {
		return;
	}

	const auto* active_paths = pathfinding.rerouting_paths.has_value() 
		? &(*pathfinding.rerouting_paths) 
		: &pathfinding.paths;

	for (const auto& path : *active_paths) {
		if (path.island_index >= navmesh.islands.size()) {
			continue;
		}

		const auto& island = navmesh.islands[path.island_index];

		for (std::size_t i = 0; i + 1 < path.nodes.size(); ++i) {
			const auto from = ::cell_to_world(island, path.nodes[i].cell_xy);
			const auto to = ::cell_to_world(island, path.nodes[i + 1].cell_xy);

			DEBUG_LOGIC_STEP_LINES.emplace_back(green, from, to);
		}
	}

	if (const auto target = ::get_current_path_target(pathfinding, navmesh)) {
		DEBUG_LOGIC_STEP_LINES.emplace_back(cyan, bot_pos, *target);
	}
}
