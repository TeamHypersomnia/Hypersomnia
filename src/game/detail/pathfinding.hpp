#pragma once
#include <optional>
#include <queue>
#include <cmath>
#include <limits>
#include "game/common_state/cosmos_navmesh.h"

/*
	Pathfinding algorithms using A* for navmesh.
*/

/*
	Convert cell coordinates to world position (center of cell).
*/

inline vec2 cell_to_world(const cosmos_navmesh_island& island, const vec2i cell_xy) {
	const auto cell_size = island.cell_size;
	const auto world_x = static_cast<float>(island.bound.l + cell_xy.x * cell_size + cell_size / 2);
	const auto world_y = static_cast<float>(island.bound.t + cell_xy.y * cell_size + cell_size / 2);
	return vec2(world_x, world_y);
}

/*
	Convert world position to cell coordinates.
*/

inline vec2i world_to_cell(const cosmos_navmesh_island& island, const vec2 world_pos) {
	const auto cell_size = island.cell_size;
	const auto cell_x = (static_cast<int>(world_pos.x) - island.bound.l) / cell_size;
	const auto cell_y = (static_cast<int>(world_pos.y) - island.bound.t) / cell_size;
	return vec2i(cell_x, cell_y);
}

/*
	Euclidean distance between two cells within an island.
*/

inline float cell_distance(const vec2i a, const vec2i b) {
	const auto dx = static_cast<float>(a.x - b.x);
	const auto dy = static_cast<float>(a.y - b.y);
	return std::sqrt(dx * dx + dy * dy);
}

/*
	Euclidean distance between two world positions.
*/

inline float world_distance(const vec2 a, const vec2 b) {
	const auto dx = a.x - b.x;
	const auto dy = a.y - b.y;
	return std::sqrt(dx * dx + dy * dy);
}

/*
	Find which island index contains a given world position.
	Returns -1 if position is not within any island.
*/

inline int find_island_for_position(const cosmos_navmesh& navmesh, const vec2 world_pos) {
	const auto pos_i = vec2i(static_cast<int>(world_pos.x), static_cast<int>(world_pos.y));

	for (std::size_t i = 0; i < navmesh.islands.size(); ++i) {
		const auto& island = navmesh.islands[i];
		const auto& bound = island.bound;

		if (pos_i.x >= bound.l && pos_i.x < bound.r &&
			pos_i.y >= bound.t && pos_i.y < bound.b) {
			return static_cast<int>(i);
		}
	}

	return -1;
}

/*
	Find the center cell of a portal within an island.
	Returns the first cell that has the portal value.
*/

inline std::optional<vec2i> find_portal_center_cell(
	const cosmos_navmesh_island& island,
	const int portal_index
) {
	const auto portal_value = static_cast<uint8_t>(2 + portal_index);
	const auto size = island.get_size_in_cells();

	vec2i sum = vec2i::zero;
	int count = 0;

	for (int cy = 0; cy < size.y; ++cy) {
		for (int cx = 0; cx < size.x; ++cx) {
			if (island.get_cell(vec2i(cx, cy)) == portal_value) {
				sum.x += cx;
				sum.y += cy;
				++count;
			}
		}
	}

	if (count == 0) {
		return std::nullopt;
	}

	return vec2i(sum.x / count, sum.y / count);
}

/*
	BFS on islands to find path from source island to target island.
	Returns the next island index to go to (-1 if no path found or same island).
*/

inline int find_islands_connection(
	const cosmos_navmesh& navmesh,
	const int source_island_index,
	const int target_island_index,
	pathfinding_context* ctx = nullptr
) {
	if (source_island_index == target_island_index) {
		return source_island_index;
	}

	if (source_island_index < 0 || target_island_index < 0) {
		return -1;
	}

	const auto num_islands = static_cast<int>(navmesh.islands.size());

	if (source_island_index >= num_islands || target_island_index >= num_islands) {
		return -1;
	}

	/*
		Use local context if none provided.
	*/
	pathfinding_context local_ctx;
	pathfinding_context& context = ctx != nullptr ? *ctx : local_ctx;

	context.islands_pathfinding_visited.clear();
	context.islands_pathfinding_visited.resize(static_cast<std::size_t>(num_islands), 0);

	/*
		BFS queue: pair of (island_index, first_step_island_index)
		first_step_island_index tracks what island we went to first from source.
	*/
	std::queue<std::pair<int, int>> bfs_queue;

	/*
		Mark source as visited.
	*/
	context.islands_pathfinding_visited[source_island_index] = 1;

	/*
		Add all islands reachable from source via portals.
	*/
	const auto& source_island = navmesh.islands[source_island_index];

	for (const auto& portal : source_island.portals) {
		const auto next_island = portal.out_island_index;

		if (next_island >= 0 && next_island < num_islands &&
			context.islands_pathfinding_visited[next_island] == 0) {
			context.islands_pathfinding_visited[next_island] = 1;

			if (next_island == target_island_index) {
				return next_island;
			}

			bfs_queue.push({ next_island, next_island });
		}
	}

	/*
		BFS.
	*/
	while (!bfs_queue.empty()) {
		const auto [current_island, first_step] = bfs_queue.front();
		bfs_queue.pop();

		const auto& island = navmesh.islands[current_island];

		for (const auto& portal : island.portals) {
			const auto next_island = portal.out_island_index;

			if (next_island >= 0 && next_island < num_islands &&
				context.islands_pathfinding_visited[next_island] == 0) {
				context.islands_pathfinding_visited[next_island] = 1;

				if (next_island == target_island_index) {
					return first_step;
				}

				bfs_queue.push({ next_island, first_step });
			}
		}
	}

	/*
		No path found.
	*/
	return -1;
}

/*
	Find best portal from source island to target island.
	Returns portal index on source island that minimizes:
	  euclidean_distance(from_pos -> portal_center) + euclidean_distance(portal_exit -> to_pos)
	
	Returns -1 if no suitable portal found.
*/

inline int find_best_portal_from_to(
	const cosmos_navmesh& navmesh,
	const int source_island_index,
	const int target_island_index,
	const vec2 from_pos,
	const vec2 to_pos
) {
	if (source_island_index < 0 || source_island_index >= static_cast<int>(navmesh.islands.size())) {
		return -1;
	}

	const auto& source_island = navmesh.islands[source_island_index];

	int best_portal_index = -1;
	auto best_distance = std::numeric_limits<float>::max();

	for (std::size_t p = 0; p < source_island.portals.size(); ++p) {
		const auto& portal = source_island.portals[p];

		/*
			For same-island portals: out_island_index == source_island_index
			For cross-island portals: out_island_index == target_island_index
		*/
		if (portal.out_island_index != target_island_index) {
			continue;
		}

		/*
			Find center of this portal.
		*/
		const auto portal_center_opt = ::find_portal_center_cell(source_island, static_cast<int>(p));

		if (!portal_center_opt.has_value()) {
			continue;
		}

		const auto portal_center_world = ::cell_to_world(source_island, portal_center_opt.value());

		/*
			Compute exit position in target island.
		*/
		vec2 exit_pos = to_pos;

		if (target_island_index >= 0 && target_island_index < static_cast<int>(navmesh.islands.size())) {
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
			best_portal_index = static_cast<int>(p);
		}
	}

	return best_portal_index;
}

/*
	A* node for priority queue.
*/

struct astar_node {
	vec2i cell;
	float g_cost;
	float f_cost;

	bool operator>(const astar_node& other) const {
		return f_cost > other.f_cost;
	}
};

/*
	A* pathfinding within a single island.
	
	target_portal_index: if >= 0, we're navigating to a specific portal.
	                     Its cells are walkable; other portal cells are blocked.
	                     If -1, we're navigating to a regular cell.

	Returns empty optional if no path found.
*/

inline std::optional<pathfinding_path> find_path_within_island(
	const cosmos_navmesh& navmesh,
	const int island_index,
	const vec2i start_cell,
	const vec2i target_cell,
	const int target_portal_index = -1,
	pathfinding_context* ctx = nullptr
) {
	if (island_index < 0 || island_index >= static_cast<int>(navmesh.islands.size())) {
		return std::nullopt;
	}

	const auto& island = navmesh.islands[island_index];
	const auto size = island.get_size_in_cells();

	if (size.x <= 0 || size.y <= 0) {
		return std::nullopt;
	}

	/*
		Check start and target are within bounds.
	*/
	if (start_cell.x < 0 || start_cell.y < 0 || start_cell.x >= size.x || start_cell.y >= size.y) {
		return std::nullopt;
	}

	if (target_cell.x < 0 || target_cell.y < 0 || target_cell.x >= size.x || target_cell.y >= size.y) {
		return std::nullopt;
	}

	/*
		Same cell.
	*/
	if (start_cell == target_cell) {
		pathfinding_path result;
		result.island_index = island_index;
		result.nodes.push_back(pathfinding_node{ start_cell });
		return result;
	}

	/*
		Use local context if none provided.
	*/
	pathfinding_context local_ctx;
	pathfinding_context& context = ctx != nullptr ? *ctx : local_ctx;

	const auto grid_size = static_cast<std::size_t>(size.x * size.y);
	context.cells_pathfinding_visited.clear();
	context.cells_pathfinding_visited.resize(grid_size, 0);

	/*
		Parent tracking for path reconstruction.
		Store parent as (x * 65536 + y + 1), where +1 avoids confusion with 0.
	*/
	std::vector<int> parent(grid_size, 0);
	std::vector<float> g_costs(grid_size, std::numeric_limits<float>::max());

	const auto target_portal_value = target_portal_index >= 0 ? static_cast<uint8_t>(2 + target_portal_index) : static_cast<uint8_t>(0);

	auto cell_index = [&](const vec2i c) {
		return static_cast<std::size_t>(c.y * size.x + c.x);
	};

	auto is_walkable = [&](const vec2i c) {
		if (c.x < 0 || c.y < 0 || c.x >= size.x || c.y >= size.y) {
			return false;
		}

		const auto value = island.get_cell(c);

		/*
			0 = free, always walkable.
		*/
		if (value == 0) {
			return true;
		}

		/*
			1 = occupied, never walkable.
		*/
		if (value == 1) {
			return false;
		}

		/*
			>= 2 = portal cells.
			Only walkable if it's the target portal.
		*/
		if (target_portal_index >= 0 && value == target_portal_value) {
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
	*/
	if (!is_walkable(start_cell)) {
		return std::nullopt;
	}

	/*
		A* priority queue.
	*/
	std::priority_queue<astar_node, std::vector<astar_node>, std::greater<astar_node>> open_set;

	const auto start_h = ::cell_distance(start_cell, target_cell);

	open_set.push({ start_cell, 0.0f, start_h });
	g_costs[cell_index(start_cell)] = 0.0f;

	/*
		8-directional neighbors.
	*/
	const vec2i directions[8] = {
		{ -1, -1 }, { 0, -1 }, { 1, -1 },
		{ -1,  0 },            { 1,  0 },
		{ -1,  1 }, { 0,  1 }, { 1,  1 }
	};

	const float dir_costs[8] = {
		1.41421356f, 1.0f, 1.41421356f,
		1.0f,              1.0f,
		1.41421356f, 1.0f, 1.41421356f
	};

	while (!open_set.empty()) {
		const auto current = open_set.top();
		open_set.pop();

		const auto current_idx = cell_index(current.cell);

		/*
			Skip if already visited with better cost.
		*/
		if (context.cells_pathfinding_visited[current_idx] != 0) {
			continue;
		}

		context.cells_pathfinding_visited[current_idx] = 1;

		/*
			Reached target.
		*/
		if (current.cell == target_cell) {
			/*
				Reconstruct path.
			*/
			pathfinding_path result;
			result.island_index = island_index;

			std::vector<vec2i> path_cells;
			auto c = target_cell;

			while (c != start_cell) {
				path_cells.push_back(c);
				const auto p = parent[cell_index(c)];

				if (p == 0) {
					/*
						Should not happen if algorithm is correct.
					*/
					break;
				}

				c = vec2i((p - 1) / size.y, (p - 1) % size.y);
			}

			path_cells.push_back(start_cell);

			/*
				Reverse to get start -> target order.
			*/
			for (auto it = path_cells.rbegin(); it != path_cells.rend(); ++it) {
				result.nodes.push_back(pathfinding_node{ *it });
			}

			return result;
		}

		/*
			Explore neighbors.
		*/
		for (int d = 0; d < 8; ++d) {
			const auto neighbor = current.cell + directions[d];

			if (!is_walkable(neighbor)) {
				continue;
			}

			const auto neighbor_idx = cell_index(neighbor);

			if (context.cells_pathfinding_visited[neighbor_idx] != 0) {
				continue;
			}

			/*
				For diagonal moves, check if both adjacent cells are walkable.
				This prevents cutting corners.
			*/
			if (directions[d].x != 0 && directions[d].y != 0) {
				const auto adj1 = current.cell + vec2i(directions[d].x, 0);
				const auto adj2 = current.cell + vec2i(0, directions[d].y);

				if (!is_walkable(adj1) || !is_walkable(adj2)) {
					continue;
				}
			}

			const auto tentative_g = current.g_cost + dir_costs[d];

			if (tentative_g < g_costs[neighbor_idx]) {
				g_costs[neighbor_idx] = tentative_g;
				parent[neighbor_idx] = current.cell.x * size.y + current.cell.y + 1;

				const auto h = ::cell_distance(neighbor, target_cell);
				open_set.push({ neighbor, tentative_g, tentative_g + h });
			}
		}
	}

	/*
		No path found.
	*/
	return std::nullopt;
}

/*
	Find path across islands with direct connection.
	Uses find_best_portal_from_to to find the best portal.
*/

inline std::optional<pathfinding_path> find_path_across_islands_direct(
	const cosmos_navmesh& navmesh,
	const int source_island_index,
	const int target_island_index,
	const vec2 source_pos,
	const vec2 target_pos,
	pathfinding_context* ctx = nullptr
) {
	if (source_island_index < 0 || source_island_index >= static_cast<int>(navmesh.islands.size())) {
		return std::nullopt;
	}

	const auto best_portal = ::find_best_portal_from_to(
		navmesh,
		source_island_index,
		target_island_index,
		source_pos,
		target_pos
	);

	if (best_portal < 0) {
		return std::nullopt;
	}

	const auto& source_island = navmesh.islands[source_island_index];
	const auto& portal = source_island.portals[best_portal];

	/*
		Find portal center cell.
	*/
	const auto portal_center_opt = ::find_portal_center_cell(source_island, best_portal);

	if (!portal_center_opt.has_value()) {
		return std::nullopt;
	}

	const auto portal_center = portal_center_opt.value();
	const auto start_cell = ::world_to_cell(source_island, source_pos);

	/*
		Find path to portal center.
	*/
	auto path_result = ::find_path_within_island(
		navmesh,
		source_island_index,
		start_cell,
		portal_center,
		best_portal,
		ctx
	);

	if (!path_result.has_value()) {
		return std::nullopt;
	}

	/*
		Set final_portal_node to indicate teleportation destination.
	*/
	path_result->final_portal_node = cell_on_island{
		portal.out_island_index,
		pathfinding_node{ portal.out_cell_pos }
	};

	return path_result;
}

/*
	Find path across multiple islands (unknown number on path).
	First does BFS on islands, then uses find_path_across_islands_direct.
*/

inline std::optional<pathfinding_path> find_path_across_islands_many(
	const cosmos_navmesh& navmesh,
	const vec2 source_pos,
	const vec2 target_pos,
	pathfinding_context* ctx = nullptr
) {
	const auto source_island = ::find_island_for_position(navmesh, source_pos);
	const auto target_island = ::find_island_for_position(navmesh, target_pos);

	if (source_island < 0 || target_island < 0) {
		return std::nullopt;
	}

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

		const auto best_same_island_portal = ::find_best_portal_from_to(
			navmesh,
			source_island,
			source_island,
			source_pos,
			target_pos
		);

		if (best_same_island_portal >= 0) {
			const auto& portal = island.portals[best_same_island_portal];
			const auto portal_center_opt = ::find_portal_center_cell(island, best_same_island_portal);

			if (portal_center_opt.has_value()) {
				const auto portal_center_world = ::cell_to_world(island, portal_center_opt.value());
				const auto exit_world = ::cell_to_world(island, portal.out_cell_pos);

				const auto portal_route_dist = ::world_distance(source_pos, portal_center_world) +
				                               ::world_distance(exit_world, target_pos);

				if (portal_route_dist < direct_dist) {
					/*
						Use portal route.
					*/
					auto path_result = ::find_path_within_island(
						navmesh,
						source_island,
						start_cell,
						portal_center_opt.value(),
						best_same_island_portal,
						ctx
					);

					if (path_result.has_value()) {
						path_result->final_portal_node = cell_on_island{
							source_island,
							pathfinding_node{ portal.out_cell_pos }
						};
						return path_result;
					}
				}
			}
		}

		/*
			Direct path within island.
		*/
		return ::find_path_within_island(
			navmesh,
			source_island,
			start_cell,
			target_cell,
			-1,
			ctx
		);
	}

	/*
		Cross-island case: find next island to go to.
	*/
	const auto next_island = ::find_islands_connection(navmesh, source_island, target_island, ctx);

	if (next_island < 0) {
		return std::nullopt;
	}

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

/*
	Helper function that repeatedly calls the main pathfinding function
	until final_portal_node is empty.
	Returns all paths concatenated for visualization.
*/

inline std::vector<pathfinding_path> find_path_across_islands_many_full(
	const cosmos_navmesh& navmesh,
	const vec2 source_pos,
	const vec2 target_pos,
	pathfinding_context* ctx = nullptr
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

		if (!path_result->final_portal_node.has_value()) {
			/*
				Reached target without needing to teleport.
			*/
			break;
		}

		/*
			Continue from the portal exit.
		*/
		const auto& portal_node = path_result->final_portal_node.value();
		const auto exit_island_idx = portal_node.island_index;

		if (exit_island_idx < 0 || exit_island_idx >= static_cast<int>(navmesh.islands.size())) {
			break;
		}

		const auto& exit_island = navmesh.islands[exit_island_idx];
		current_pos = ::cell_to_world(exit_island, portal_node.node.cell_xy);
	}

	return all_paths;
}
