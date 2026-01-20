#include <limits>
#include <queue>
#include "game/detail/pathfinding.h"
#include "game/detail/pathfinding_graph_view.h"
#include "augs/algorithm/bfs.hpp"
#include "augs/algorithm/a_star.hpp"
#include "augs/templates/reversion_wrapper.h"
#include "augs/misc/randomization.h"
#include "game/inferred_caches/physics_world_cache.h"
#include "game/enums/filters.h"
#include "game/detail/physics/physics_queries.h"
#include "augs/math/transform.h"
#include "augs/math/repro_math.h"

/*
	Helper function to check if a point is inside a rotated rectangle.
*/

static bool point_in_rotated_rect(
	const vec2 point,
	const transformr& rect_transform,
	const vec2 rect_half_size
) {
	const auto local_point = (point - rect_transform.pos).rotate(-rect_transform.rotation, vec2::zero);
	return repro::fabs(local_point.x) <= rect_half_size.x && repro::fabs(local_point.y) <= rect_half_size.y;
}

vec2 cell_to_world(const cosmos_navmesh_island& island, const vec2u cell_xy) {
	const auto cell_size = static_cast<float>(island.cell_size);
	return vec2(island.bound.lt()) + vec2(cell_xy) * cell_size + vec2::square(cell_size / 2);
}

vec2u world_to_cell(const cosmos_navmesh_island& island, const vec2 world_pos) {
	const auto offset = world_pos - vec2(island.bound.lt());
	const auto cell_size = static_cast<float>(island.cell_size);
	return vec2u(
		static_cast<uint32_t>(std::max(0.0f, offset.x) / cell_size),
		static_cast<uint32_t>(std::max(0.0f, offset.y) / cell_size)
	);
}

uint32_t cell_distance(const vec2u a, const vec2u b) {
	const auto dx = a.x > b.x ? a.x - b.x : b.x - a.x;
	const auto dy = a.y > b.y ? a.y - b.y : b.y - a.y;
	return dx + dy;
}

/*
	Euclidean distance between two cells (for 8-directional A*).
*/

float cell_distance_euclidean(const vec2u a, const vec2u b) {
	const auto dx = static_cast<float>(a.x > b.x ? a.x - b.x : b.x - a.x);
	const auto dy = static_cast<float>(a.y > b.y ? a.y - b.y : b.y - a.y);
	return repro::sqrt(dx * dx + dy * dy);
}

float world_distance(const vec2 a, const vec2 b) {
	return (a - b).length();
}

std::optional<island_id_type> find_island_for_position(const cosmos_navmesh& navmesh, const vec2 world_pos) {
	const auto pos_i = vec2i(world_pos);

	for (island_id_type i = 0; i < navmesh.islands.size(); ++i) {
		const auto& island = navmesh.islands[i];

		if (island.bound.hover(pos_i)) {
			return i;
		}
	}

	return std::nullopt;
}

std::optional<island_id_type> find_islands_connection(
	const cosmos_navmesh& navmesh,
	const island_id_type source_island_index,
	const island_id_type target_island_index,
	pathfinding_context* ctx
) {
	if (source_island_index == target_island_index) {
		return source_island_index;
	}

	const auto num_islands = static_cast<island_id_type>(navmesh.islands.size());

	if (source_island_index >= num_islands || target_island_index >= num_islands) {
		return std::nullopt;
	}

	/*
		Use local context if none provided.
	*/
	pathfinding_context local_ctx;
	pathfinding_context& context = ctx != nullptr ? *ctx : local_ctx;

	context.islands_pathfinding_visited.clear();
	context.islands_pathfinding_visited.resize(num_islands, 0);

	auto get_visited = [&](const island_id_type island_idx) {
		return context.islands_pathfinding_visited[island_idx] != 0;
	};

	auto set_visited = [&](const island_id_type island_idx) {
		context.islands_pathfinding_visited[island_idx] = 1;
	};

	auto for_each_neighbor = [&](const island_id_type island_idx, auto&& callback) {
		const auto& island = navmesh.islands[island_idx];

		for (const auto& portal : island.portals) {
			const auto next_island = static_cast<island_id_type>(portal.out_island_index);

			if (portal.out_island_index >= 0 && next_island < num_islands) {
				if (callback(next_island) == callback_result::ABORT) {
					return;
				}
			}
		}
	};

	return augs::bfs_find_next_edge(
		context.bfs_queue,
		source_island_index,
		target_island_index,
		get_visited,
		set_visited,
		for_each_neighbor
	);
}

std::optional<std::size_t> find_best_portal_from_to(
	const cosmos_navmesh& navmesh,
	const island_id_type source_island_index,
	const island_id_type target_island_index,
	const vec2 from_pos,
	const vec2 to_pos
) {
	if (source_island_index >= navmesh.islands.size()) {
		return std::nullopt;
	}

	const auto& source_island = navmesh.islands[source_island_index];

	std::optional<std::size_t> best_portal_index;
	auto best_distance = std::numeric_limits<float>::max();

	for (std::size_t p = 0; p < source_island.portals.size(); ++p) {
		const auto& portal = source_island.portals[p];

		/*
			For same-island portals: out_island_index == source_island_index
			For cross-island portals: out_island_index == target_island_index
		*/
		if (static_cast<island_id_type>(portal.out_island_index) != target_island_index) {
			continue;
		}

		/*
			Use pre-computed in_cell_pos for this portal.
		*/
		const auto portal_center_world = ::cell_to_world(source_island, portal.in_cell_pos);

		/*
			Compute exit position in target island.
		*/
		vec2 exit_pos = to_pos;

		if (target_island_index < navmesh.islands.size()) {
			const auto& target_island = navmesh.islands[target_island_index];
			exit_pos = ::cell_to_world(target_island, portal.out_cell_pos);
		}

		/*
			Total distance: from_pos -> portal + portal_exit -> to_pos
		*/
		const auto dist_to_portal = ::world_distance(from_pos, portal_center_world);
		const auto dist_from_exit = ::world_distance(exit_pos, to_pos);
		const auto total_dist = dist_to_portal + dist_from_exit;

		if (total_dist < best_distance) {
			best_distance = total_dist;
			best_portal_index = p;
		}
	}

	return best_portal_index;
}

std::optional<std::vector<pathfinding_node>> find_path_within_island(
	const cosmos_navmesh_island& island,
	const vec2u start_cell,
	const vec2u target_cell,
	const std::optional<std::size_t> target_portal_index,
	pathfinding_context* ctx
) {
	const auto size = island.get_size_in_cells();

	if (size.x == 0 || size.y == 0) {
		return std::nullopt;
	}

	/*
		Check start and target are within bounds.
	*/
	if (start_cell.x >= size.x || start_cell.y >= size.y) {
		return std::nullopt;
	}

	if (target_cell.x >= size.x || target_cell.y >= size.y) {
		return std::nullopt;
	}

	/*
		Same cell.
	*/
	if (start_cell == target_cell) {
		std::vector<pathfinding_node> result;
		result.push_back(pathfinding_node{ start_cell });
		return result;
	}

	/*
		Use local context if none provided.
	*/
	pathfinding_context local_ctx;
	pathfinding_context& context = ctx != nullptr ? *ctx : local_ctx;

	pathfinding_graph_view graph(context, island);
	graph.init_all();

	std::optional<uint8_t> source_portal_value;

	if (const auto start_value = island.get_cell(start_cell); ::is_cell_portal(start_value)) {
		source_portal_value = start_value;
	}

	auto is_walkable = [&](const vec2u c) {
		if (c.x >= graph.size.x || c.y >= graph.size.y) {
			return false;
		}

		const auto value = island.get_cell(c);

		if (::is_cell_unoccupied(value)) {
			return true;
		}

		if (::is_cell_unwalkable(value)) {
			return false;
		}

		/*
			Portal cells (>= 2) are walkable only if it's the target or source portal.
		*/
		if (::is_cell_target_portal(value, target_portal_index)) {
			return true;
		}

		if (value == source_portal_value) {
			return true;
		}

		return false;
	};

	/*
		Check if target is reachable.
	*/
	if (!is_walkable(target_cell)) {
		return std::nullopt;
	}

	/*
		Check if start is walkable.
		If not, find the closest unoccupied cell and prepend the path through occupied cells.
	*/
	std::vector<pathfinding_node> prefix_path;
	vec2u actual_start = start_cell;

	if (!is_walkable(start_cell)) {
		const auto start_world = ::cell_to_world(island, start_cell);
		const auto unoccupied_result = ::find_closest_unoccupied_cell(island, start_cell, start_world, ctx);

		if (!unoccupied_result.has_value()) {
			return std::nullopt;
		}

		prefix_path = unoccupied_result->path_through_occupied;
		actual_start = unoccupied_result->cell;

		/*
			Re-initialize graph since find_closest_unoccupied_cell may have modified context.
		*/
		graph.init_all();
	}

	/*
		If actual_start == target, return prefix path + target.
	*/
	if (actual_start == target_cell) {
		prefix_path.push_back(pathfinding_node{ target_cell });
		return prefix_path;
	}

	auto heuristic = [&](const vec2u c) {
		return ::cell_distance_euclidean(c, target_cell);
	};

	auto is_target = [&](const vec2u c) {
		return c == target_cell;
	};

	const auto found = augs::astar_find_path_weighted(
		context.astar_queue,
		actual_start,
		graph.make_get_visited(),
		graph.make_set_visited(),
		graph.make_for_each_neighbor_8_with_weight(is_walkable),
		heuristic,
		is_target,
		graph.make_set_parent(),
		graph.make_get_g_cost(),
		graph.make_set_g_cost()
	);

	if (!found) {
		return std::nullopt;
	}

	auto main_path = graph.reconstruct_path(actual_start, target_cell);

	/*
		Prepend the path through occupied cells if any.
	*/
	if (!prefix_path.empty()) {
		/*
			Remove the last element of prefix_path since it's the same as first of main_path.
		*/
		prefix_path.pop_back();
		prefix_path.insert(prefix_path.end(), main_path.begin(), main_path.end());
		return prefix_path;
	}

	return main_path;
}

std::optional<pathfinding_path> find_path_across_islands_direct(
	const cosmos_navmesh& navmesh,
	const island_id_type source_island_index,
	const island_id_type target_island_index,
	const vec2 source_pos,
	const vec2 target_pos,
	pathfinding_context* ctx
) {
	if (source_island_index >= navmesh.islands.size()) {
		return std::nullopt;
	}

	const auto best_portal_opt = ::find_best_portal_from_to(
		navmesh,
		source_island_index,
		target_island_index,
		source_pos,
		target_pos
	);

	if (!best_portal_opt.has_value()) {
		return std::nullopt;
	}

	const auto best_portal = best_portal_opt.value();
	const auto& source_island = navmesh.islands[source_island_index];
	const auto& portal = source_island.portals[best_portal];

	/*
		Use pre-computed in_cell_pos for portal center.
	*/
	const auto portal_center = portal.in_cell_pos;
	const auto start_cell = ::world_to_cell(source_island, source_pos);

	/*
		Find path to portal center.
	*/
	auto path_nodes = ::find_path_within_island(
		source_island,
		start_cell,
		portal_center,
		best_portal,
		ctx
	);

	if (!path_nodes.has_value()) {
		return std::nullopt;
	}

	pathfinding_path result;
	result.island_index = source_island_index;
	result.nodes = std::move(path_nodes.value());

	/*
		Set final_portal_exit to indicate teleportation destination.
	*/
	result.final_portal_exit = cell_on_navmesh(
		static_cast<island_id_type>(portal.out_island_index),
		portal.out_cell_pos
	);

	return result;
}

std::optional<pathfinding_path> find_path_across_islands_many(
	const cosmos_navmesh& navmesh,
	const vec2 source_pos,
	const vec2 target_pos,
	pathfinding_context* ctx
) {
	const auto source_island_opt = ::find_island_for_position(navmesh, source_pos);
	const auto target_island_opt = ::find_island_for_position(navmesh, target_pos);

	if (!source_island_opt.has_value() || !target_island_opt.has_value()) {
		return std::nullopt;
	}

	const auto source_island = source_island_opt.value();
	const auto target_island = target_island_opt.value();

	/*
		Same island case.
	*/
	if (source_island == target_island) {
		const auto& island = navmesh.islands[source_island];
		const auto start_cell = ::world_to_cell(island, source_pos);
		const auto target_cell = ::world_to_cell(island, target_pos);

		/*
			Check for same-island portals that might provide a shortcut.
		*/
		const auto direct_dist = ::world_distance(source_pos, target_pos);

		const auto best_same_island_portal_opt = ::find_best_portal_from_to(
			navmesh,
			source_island,
			source_island,
			source_pos,
			target_pos
		);

		if (best_same_island_portal_opt.has_value()) {
			const auto best_same_island_portal = best_same_island_portal_opt.value();
			const auto& portal = island.portals[best_same_island_portal];

			const auto portal_center_world = ::cell_to_world(island, portal.in_cell_pos);
			const auto exit_world = ::cell_to_world(island, portal.out_cell_pos);

			const auto portal_route_dist = ::world_distance(source_pos, portal_center_world) +
			                               ::world_distance(exit_world, target_pos);

			if (portal_route_dist < direct_dist) {
				/*
					Use portal route.
				*/
				auto path_nodes = ::find_path_within_island(
					island,
					start_cell,
					portal.in_cell_pos,
					best_same_island_portal,
					ctx
				);

				if (path_nodes.has_value()) {
					pathfinding_path result;
					result.island_index = source_island;
					result.nodes = std::move(path_nodes.value());
					result.final_portal_exit = cell_on_navmesh(
						source_island,
						portal.out_cell_pos
					);
					return result;
				}
			}
		}

		/*
			Direct path within island.
		*/
		auto path_nodes = ::find_path_within_island(
			island,
			start_cell,
			target_cell,
			std::nullopt,
			ctx
		);

		if (!path_nodes.has_value()) {
			return std::nullopt;
		}

		pathfinding_path result;
		result.island_index = source_island;
		result.nodes = std::move(path_nodes.value());
		return result;
	}

	/*
		Cross-island case: find next island to go to.
	*/
	const auto next_island_opt = ::find_islands_connection(navmesh, source_island, target_island, ctx);

	if (!next_island_opt.has_value()) {
		return std::nullopt;
	}

	const auto next_island = next_island_opt.value();

	/*
		Find path to portal leading to next island.
	*/
	return ::find_path_across_islands_direct(
		navmesh,
		source_island,
		next_island,
		source_pos,
		target_pos,
		ctx
	);
}

std::vector<pathfinding_path> find_path_across_islands_many_full(
	const cosmos_navmesh& navmesh,
	const vec2 source_pos,
	const vec2 target_pos,
	pathfinding_context* ctx
) {
	std::vector<pathfinding_path> all_paths;
	auto current_pos = source_pos;

	/*
		Safety limit to prevent infinite loops.
	*/
	const int max_iterations = 100;

	for (int iteration = 0; iteration < max_iterations; ++iteration) {
		auto path_result = ::find_path_across_islands_many(navmesh, current_pos, target_pos, ctx);

		if (!path_result.has_value()) {
			break;
		}

		all_paths.push_back(path_result.value());

		if (!path_result->final_portal_exit.has_value()) {
			/*
				Reached target without needing to teleport.
			*/
			break;
		}

		/*
			Continue from the portal exit.
		*/
		const auto& portal_node = path_result->final_portal_exit.value();
		const auto exit_island_idx = portal_node.island_index;

		if (exit_island_idx >= navmesh.islands.size()) {
			break;
		}

		const auto& exit_island = navmesh.islands[exit_island_idx];
		current_pos = ::cell_to_world(exit_island, portal_node.node.cell_xy);
	}

	return all_paths;
}

std::optional<unoccupied_cell_result> find_closest_unoccupied_cell(
	const cosmos_navmesh_island& island,
	const vec2u start_cell,
	const vec2 world_pos,
	pathfinding_context* ctx
) {
	const auto size = island.get_size_in_cells();

	if (size.x == 0 || size.y == 0) {
		return std::nullopt;
	}

	if (start_cell.x >= size.x || start_cell.y >= size.y) {
		return std::nullopt;
	}

	/*
		If start cell is already unoccupied, return it immediately.
	*/
	if (island.is_cell_unoccupied(start_cell)) {
		unoccupied_cell_result result;
		result.cell = start_cell;
		return result;
	}

	pathfinding_context local_ctx;
	pathfinding_context& context = ctx != nullptr ? *ctx : local_ctx;

	pathfinding_graph_view graph(context, island);
	graph.init_visited();
	graph.init_parent();

	/*
		A cell is a valid target if it's unoccupied (value == 0).
	*/
	auto is_target_cell = [&](const vec2u c) {
		return island.is_cell_unoccupied(c);
	};

	/*
		Use BFS to find up to MAX_WALKABLE_CANDIDATES unoccupied cells,
		tracking the closest one by Euclidean distance.
		BFS iterates all in-bounds neighbors (regardless of walkability).
	*/
	std::vector<vec2u> candidates;
	candidates.reserve(MAX_WALKABLE_CANDIDATES);

	augs::bfs_full(
		start_cell,
		graph.make_get_visited(),
		graph.make_set_visited(),
		graph.make_set_parent(),
		graph.make_for_each_neighbor_all(),
		is_target_cell,
		[&](const vec2u cell) {
			candidates.push_back(cell);

			if (candidates.size() >= MAX_WALKABLE_CANDIDATES) {
				return callback_result::ABORT;
			}

			return callback_result::CONTINUE;
		}
	);

	if (candidates.empty()) {
		return std::nullopt;
	}

	/*
		Find the candidate with minimum Euclidean distance to world_pos.
	*/
	vec2u best_cell = candidates[0];
	float best_dist_sq = (::cell_to_world(island, best_cell) - world_pos).length_sq();

	for (std::size_t i = 1; i < candidates.size(); ++i) {
		const auto cell_world = ::cell_to_world(island, candidates[i]);
		const auto dist_sq = (cell_world - world_pos).length_sq();

		if (dist_sq < best_dist_sq) {
			best_dist_sq = dist_sq;
			best_cell = candidates[i];
		}
	}

	unoccupied_cell_result result;
	result.cell = best_cell;
	result.path_through_occupied = graph.reconstruct_path(start_cell, best_cell);

	return result;
}

std::optional<vec2u> find_random_unoccupied_cell_within_steps(
	const cosmos_navmesh_island& island,
	const vec2u start_cell,
	const uint32_t max_steps,
	randomization& rng,
	pathfinding_context* ctx
) {
	const auto size = island.get_size_in_cells();

	if (size.x == 0 || size.y == 0) {
		return std::nullopt;
	}

	if (start_cell.x >= size.x || start_cell.y >= size.y) {
		return std::nullopt;
	}

	/*
		First, find an unoccupied starting cell if the start is occupied.
	*/
	vec2u current = start_cell;

	if (!island.is_cell_unoccupied(start_cell)) {
		const auto world_pos = ::cell_to_world(island, start_cell);
		const auto unoccupied = ::find_closest_unoccupied_cell(island, start_cell, world_pos, ctx);

		if (!unoccupied.has_value()) {
			return std::nullopt;
		}

		current = unoccupied->cell;
	}

	int came_from_dir = -1;

	for (uint32_t step = 0; step < max_steps; ++step) {
		/*
			Collect valid directions (unoccupied and not the direction we came from).
		*/
		std::vector<int> valid_dirs;

		for (int d = 0; d < 4; ++d) {
			if (step > 0 && d == came_from_dir) {
				continue;
			}

			const auto dir = CELL_DIRECTIONS[d];
			const auto neighbor = vec2u(vec2i(current) + dir);

			if (island.is_cell_unoccupied(neighbor)) {
				valid_dirs.push_back(d);
			}
		}

		if (valid_dirs.empty()) {
			break;
		}

		const auto chosen_idx = rng.randval(0u, static_cast<unsigned>(valid_dirs.size() - 1));
		const auto chosen_dir = valid_dirs[chosen_idx];

		const auto dir = CELL_DIRECTIONS[chosen_dir];
		current = vec2u(vec2i(current) + dir);

		/*
			Calculate the opposite direction.
		*/
		came_from_dir = (chosen_dir + 2) % 4;
	}

	return current;
}

std::optional<vec2> find_random_unoccupied_position_within_steps(
	const cosmos_navmesh& navmesh,
	const vec2 world_pos,
	const uint32_t max_steps,
	randomization& rng,
	pathfinding_context* ctx
) {
	const auto island_opt = ::find_island_for_position(navmesh, world_pos);

	if (!island_opt.has_value()) {
		return std::nullopt;
	}

	const auto island_index = *island_opt;
	const auto& island = navmesh.islands[island_index];
	const auto start_cell = ::world_to_cell(island, world_pos);

	const auto result_cell = ::find_random_unoccupied_cell_within_steps(island, start_cell, max_steps, rng, ctx);

	if (!result_cell.has_value()) {
		return std::nullopt;
	}

	return ::cell_to_world(island, *result_cell);
}

std::optional<vec2> find_random_unoccupied_cell_within_rect(
	const cosmos_navmesh& navmesh,
	const ltrb& aabb,
	const transformr& rect_transform,
	const vec2 rect_size,
	randomization& rng
) {
	/*
		Find which island(s) the AABB overlaps and collect candidate cells.
	*/
	const auto rect_half_size = rect_size / 2.0f;
	std::vector<vec2> candidate_cells;

	for (island_id_type island_idx = 0; island_idx < navmesh.islands.size(); ++island_idx) {
		const auto& island = navmesh.islands[island_idx];

		/*
			Check if island overlaps with AABB.
		*/
		if (!island.bound.hover(ltrbi(aabb))) {
			continue;
		}

		const auto island_size = island.get_size_in_cells();

		/*
			Calculate cell range that overlaps with AABB.
		*/
		const auto min_cell = ::world_to_cell(island, vec2(aabb.l, aabb.t));
		const auto max_cell_raw = ::world_to_cell(island, vec2(aabb.r, aabb.b));
		const auto max_cell = vec2u(
			std::min(max_cell_raw.x, island_size.x > 0 ? island_size.x - 1 : 0),
			std::min(max_cell_raw.y, island_size.y > 0 ? island_size.y - 1 : 0)
		);

		/*
			Skip if the cell range is invalid (min > max).
		*/
		if (min_cell.x > max_cell.x || min_cell.y > max_cell.y) {
			continue;
		}

		/*
			Iterate over cells in the range.
		*/
		for (uint32_t y = min_cell.y; y <= max_cell.y; ++y) {
			for (uint32_t x = min_cell.x; x <= max_cell.x; ++x) {
				const auto cell_pos = vec2u(x, y);

				if (!island.is_cell_unoccupied(cell_pos)) {
					continue;
				}

				const auto world_pos = ::cell_to_world(island, cell_pos);

				if (::point_in_rotated_rect(world_pos, rect_transform, rect_half_size)) {
					candidate_cells.push_back(world_pos);
				}
			}
		}
	}

	if (candidate_cells.empty()) {
		return std::nullopt;
	}

	const auto random_index = rng.randval(0u, static_cast<unsigned>(candidate_cells.size() - 1));
	return candidate_cells[random_index];
}
