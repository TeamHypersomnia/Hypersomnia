#include <limits>
#include "game/detail/pathfinding.h"
#include "augs/algorithm/bfs.hpp"
#include "augs/algorithm/a_star.hpp"
#include "augs/templates/reversion_wrapper.h"

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

float world_distance(const vec2 a, const vec2 b) {
	return (a - b).length();
}

std::optional<std::size_t> find_island_for_position(const cosmos_navmesh& navmesh, const vec2 world_pos) {
	const auto pos_i = vec2i(world_pos);

	for (std::size_t i = 0; i < navmesh.islands.size(); ++i) {
		const auto& island = navmesh.islands[i];

		if (island.bound.hover(pos_i)) {
			return i;
		}
	}

	return std::nullopt;
}

std::optional<std::size_t> find_islands_connection(
	const cosmos_navmesh& navmesh,
	const std::size_t source_island_index,
	const std::size_t target_island_index,
	pathfinding_context* ctx
) {
	if (source_island_index == target_island_index) {
		return source_island_index;
	}

	const auto num_islands = navmesh.islands.size();

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

	auto get_visited = [&](const std::size_t island_idx) {
		return context.islands_pathfinding_visited[island_idx] != 0;
	};

	auto set_visited = [&](const std::size_t island_idx) {
		context.islands_pathfinding_visited[island_idx] = 1;
	};

	auto for_each_neighbor = [&](const std::size_t island_idx, auto&& callback) {
		const auto& island = navmesh.islands[island_idx];

		for (const auto& portal : island.portals) {
			const auto next_island = static_cast<std::size_t>(portal.out_island_index);

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
	const std::size_t source_island_index,
	const std::size_t target_island_index,
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
		if (static_cast<std::size_t>(portal.out_island_index) != target_island_index) {
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

	const auto grid_size = static_cast<std::size_t>(size.area());
	context.cells_pathfinding_visited.clear();
	context.cells_pathfinding_visited.resize(grid_size, 0);

	/*
		Parent tracking for path reconstruction.
		-1 means unset, 0 is the first valid cell.
	*/
	context.cells_parent.clear();
	context.cells_parent.resize(grid_size, -1);
	context.cells_g_costs.clear();
	context.cells_g_costs.resize(grid_size, std::numeric_limits<float>::max());

	const auto target_portal_value = target_portal_index.has_value() 
		? static_cast<uint8_t>(2 + target_portal_index.value()) 
		: static_cast<uint8_t>(0);

	auto get_cell_index = [&](const vec2u c) {
		return island.cell_index(c);
	};

	std::optional<std::size_t> source_portal_index;

	if (const auto start_type = island.get_cell(start_cell); start_type >= 2) {
		source_portal_index = start_type;
	}

	auto is_walkable = [&](const vec2u c) {
		if (c.x >= size.x || c.y >= size.y) {
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
		if (target_portal_index.has_value() && value == target_portal_value) {
			return true;
		}

		if (value == source_portal_index) {
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
		4-directional neighbors (up, down, left, right only).
		Use signed offsets to handle going left/up.
	*/
	const vec2i directions[4] = {
		{ 0, -1 },
		{ 0,  1 },
		{ -1, 0 },
		{ 1,  0 }
	};

	auto get_visited = [&](const vec2u c) {
		return context.cells_pathfinding_visited[get_cell_index(c)] != 0;
	};

	auto set_visited = [&](const vec2u c) {
		context.cells_pathfinding_visited[get_cell_index(c)] = 1;
	};

	auto for_each_neighbor = [&](const vec2u c, auto&& callback) {
		for (uint32_t d = 0; d < 4; ++d) {
			const auto dir = directions[d];

			/*
				Check for underflow when going left/up.
			*/
			if (dir.x < 0 && c.x == 0) {
				continue;
			}
			if (dir.y < 0 && c.y == 0) {
				continue;
			}

			const auto neighbor = vec2u(
				static_cast<uint32_t>(static_cast<int>(c.x) + dir.x),
				static_cast<uint32_t>(static_cast<int>(c.y) + dir.y)
			);

			if (is_walkable(neighbor)) {
				if (callback(neighbor) == callback_result::ABORT) {
					return;
				}
			}
		}
	};

	auto heuristic = [&](const vec2u c) {
		return static_cast<float>(::cell_distance(c, target_cell));
	};

	auto is_target = [&](const vec2u c) {
		return c == target_cell;
	};

	auto get_parent = [&](const vec2u c) -> std::optional<vec2u> {
		const auto p = context.cells_parent[get_cell_index(c)];
		if (p < 0) {
			return std::nullopt;
		}
		return vec2u(static_cast<uint32_t>(p) % size.x, static_cast<uint32_t>(p) / size.x);
	};

	auto set_parent = [&](const vec2u child, const vec2u parent_cell) {
		context.cells_parent[get_cell_index(child)] = static_cast<int>(parent_cell.y * size.x + parent_cell.x);
	};

	auto get_g_cost = [&](const vec2u c) {
		return context.cells_g_costs[get_cell_index(c)];
	};

	auto set_g_cost = [&](const vec2u c, const float cost) {
		context.cells_g_costs[get_cell_index(c)] = cost;
	};

	const auto found = augs::astar_find_path(
		context.astar_queue,
		start_cell,
		get_visited,
		set_visited,
		for_each_neighbor,
		heuristic,
		is_target,
		set_parent,
		get_g_cost,
		set_g_cost
	);

	if (!found) {
		return std::nullopt;
	}

	/*
		Reconstruct path.
	*/
	std::vector<vec2u> path_cells;
	auto c = target_cell;

	while (c != start_cell) {
		path_cells.push_back(c);
		const auto p = get_parent(c);

		if (!p.has_value()) {
			/*
				Should not happen if algorithm is correct.
			*/
			break;
		}

		c = p.value();
	}

	path_cells.push_back(start_cell);

	/*
		Reverse to get start -> target order.
	*/
	std::vector<pathfinding_node> result;
	for (const auto& cell : reverse(path_cells)) {
		result.push_back(pathfinding_node{ cell });
	}

	return result;
}

std::optional<pathfinding_path> find_path_across_islands_direct(
	const cosmos_navmesh& navmesh,
	const std::size_t source_island_index,
	const std::size_t target_island_index,
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
		Set final_portal_node to indicate teleportation destination.
	*/
	result.final_portal_node = cell_on_island{
		static_cast<std::size_t>(portal.out_island_index),
		pathfinding_node{ portal.out_cell_pos }
	};

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
					result.final_portal_node = cell_on_island{
						source_island,
						pathfinding_node{ portal.out_cell_pos }
					};
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

		if (exit_island_idx >= navmesh.islands.size()) {
			break;
		}

		const auto& exit_island = navmesh.islands[exit_island_idx];
		current_pos = ::cell_to_world(exit_island, portal_node.node.cell_xy);
	}

	return all_paths;
}
