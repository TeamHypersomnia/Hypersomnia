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

inline void advance_path_if_reached(
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
		const auto cell_size = static_cast<float>(island.cell_size);

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
		    pathfinding.rerouting->node_index >= pathfinding.rerouting->path.nodes.size()) {
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
*/

inline void check_path_deviation(
	ai_pathfinding_state& pathfinding,
	const vec2 bot_pos,
	const cosmos_navmesh& navmesh,
	pathfinding_context* ctx
) {
	const auto& main_path = pathfinding.main.path;

	/*
		If already rerouting, check if we fell off the rerouting path.
	*/
	if (pathfinding.rerouting.has_value()) {
		/*
			Recalculate rerouting to the main path cell.
		*/
		if (main_path.island_index >= navmesh.islands.size() ||
		    pathfinding.main.node_index >= main_path.nodes.size()) {
			return;
		}

		const auto& target_island = navmesh.islands[main_path.island_index];
		const auto target_cell = main_path.nodes[pathfinding.main.node_index].cell_xy;
		const auto target_world = ::cell_to_world(target_island, target_cell);

		auto new_rerouting = ::find_path_across_islands_many(navmesh, bot_pos, target_world, ctx);

		if (new_rerouting.has_value()) {
			pathfinding.rerouting = pathfinding_progress{
				std::move(*new_rerouting),
				0
			};
		}

		return;
	}

	/*
		Check if we're on any of the nearby path cells.
	*/
	if (main_path.island_index >= navmesh.islands.size()) {
		return;
	}

	const auto& island = navmesh.islands[main_path.island_index];
	const auto& nodes = main_path.nodes;
	const auto current_idx = pathfinding.main.node_index;

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
		pathfinding.main.node_index = *closest_within_bounds;
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
	auto rerouting_path = ::find_path_across_islands_many(navmesh, bot_pos, reroute_target_world, ctx);

	if (rerouting_path.has_value()) {
		pathfinding.rerouting = pathfinding_progress{
			std::move(*rerouting_path),
			0
		};
		pathfinding.main.node_index = reroute_target_idx;
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
		Don't initiate new pathfinding while standing on a portal cell.
		Existing pathfinding sessions can continue, but new ones should wait
		until the bot has teleported through the portal.
	*/
	if (!ai_state.is_pathfinding_active() && ::is_on_portal_cell(bot_pos, navmesh)) {
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

	if (ai_state.is_pathfinding_active() &&
	    ai_state.pathfinding->target_island == target_island &&
	    ai_state.pathfinding->target_cell == target_cell) {
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

	ai_state.pathfinding = ai_pathfinding_state{
		pathfinding_progress{ std::move(*path), 0 },
		std::nullopt,
		target_pos,
		target_island,
		target_cell
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

	if (pathfinding_opt.has_value() &&
	    pathfinding_opt->target_island == target_island &&
	    pathfinding_opt->target_cell == target_cell) {
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
		target_island,
		target_cell
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
		return dir.normalize();
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

	return dir.normalize();
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

	auto draw_path = [&](const pathfinding_progress& progress) {
		const auto& path = progress.path;

		if (path.island_index >= navmesh.islands.size()) {
			return;
		}

		const auto& island = navmesh.islands[path.island_index];

		for (std::size_t i = 0; i + 1 < path.nodes.size(); ++i) {
			const auto from = ::cell_to_world(island, path.nodes[i].cell_xy);
			const auto to = ::cell_to_world(island, path.nodes[i + 1].cell_xy);

			DEBUG_LOGIC_STEP_LINES.emplace_back(green, from, to);
		}
	};

	draw_path(pathfinding.main);

	if (pathfinding.rerouting.has_value()) {
		draw_path(*pathfinding.rerouting);
	}

	if (const auto target = ::get_current_path_target(pathfinding, navmesh)) {
		DEBUG_LOGIC_STEP_LINES.emplace_back(cyan, bot_pos, *target);
	}
}
