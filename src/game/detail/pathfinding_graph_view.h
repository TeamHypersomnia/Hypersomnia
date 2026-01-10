#pragma once
#include "game/common_state/cosmos_navmesh.h"
#include "game/common_state/cosmos_pathfinding.h"
#include "augs/enums/callback_result.h"

/*
	pathfinding_graph_view consolidates the repeated graph traversal logic
	(get_visited, set_visited, set_parent, get_parent, for_each_neighbor)
	used across pathfinding functions.

	It provides a uniform interface over a pathfinding_context and island,
	with a customizable walkability predicate.
*/

struct pathfinding_graph_view {
	pathfinding_context& context;
	const cosmos_navmesh_island& island;
	const vec2u size;

	pathfinding_graph_view(
		pathfinding_context& ctx,
		const cosmos_navmesh_island& isl
	) : context(ctx)
	  , island(isl)
	  , size(isl.get_size_in_cells())
	{}

	void init_visited() {
		const auto grid_size = static_cast<std::size_t>(size.area());
		context.cells_pathfinding_visited.clear();
		context.cells_pathfinding_visited.resize(grid_size, 0);
	}

	void init_parent() {
		const auto grid_size = static_cast<std::size_t>(size.area());
		context.cells_parent.clear();
		context.cells_parent.resize(grid_size, -1);
	}

	void init_g_costs() {
		const auto grid_size = static_cast<std::size_t>(size.area());
		context.cells_g_costs.clear();
		context.cells_g_costs.resize(grid_size, std::numeric_limits<float>::max());
	}

	void init_all() {
		init_visited();
		init_parent();
		init_g_costs();
	}

	std::size_t cell_index(const vec2u c) const {
		return island.cell_index(c);
	}

	bool get_visited(const vec2u c) const {
		return context.cells_pathfinding_visited[cell_index(c)] != 0;
	}

	void set_visited(const vec2u c) {
		context.cells_pathfinding_visited[cell_index(c)] = 1;
	}

	std::optional<vec2u> get_parent(const vec2u c) const {
		const auto p = context.cells_parent[cell_index(c)];

		if (p < 0) {
			return std::nullopt;
		}

		return vec2u(static_cast<uint32_t>(p) % size.x, static_cast<uint32_t>(p) / size.x);
	}

	void set_parent(const vec2u child, const vec2u parent_cell) {
		context.cells_parent[cell_index(child)] = static_cast<int>(parent_cell.y * size.x + parent_cell.x);
	}

	float get_g_cost(const vec2u c) const {
		return context.cells_g_costs[cell_index(c)];
	}

	void set_g_cost(const vec2u c, const float cost) {
		context.cells_g_costs[cell_index(c)] = cost;
	}

	/*
		Iterate over 4-directional neighbors of cell c,
		calling callback for each that passes the filter predicate.
	*/
	template <class Filter, class Callback>
	void for_each_neighbor(const vec2u c, Filter&& filter, Callback&& callback) const {
		for (uint32_t d = 0; d < 4; ++d) {
			const auto dir = CELL_DIRECTIONS[d];
			const auto neighbor = vec2u(vec2i(c) + dir);

			if (filter(neighbor)) {
				if (callback(neighbor) == callback_result::ABORT) {
					return;
				}
			}
		}
	}

	/*
		Reconstruct path from start_cell to target_cell using stored parent pointers.
		Returns path in start -> target order.
	*/
	std::vector<pathfinding_node> reconstruct_path(const vec2u start_cell, const vec2u target_cell) const {
		std::vector<vec2u> path_cells;
		auto c = target_cell;

		while (c != start_cell) {
			path_cells.push_back(c);
			const auto p = get_parent(c);

			if (!p.has_value()) {
				break;
			}

			c = *p;
		}

		path_cells.push_back(start_cell);

		/*
			Reverse to get start -> target order.
		*/
		std::vector<pathfinding_node> result;

		for (auto it = path_cells.rbegin(); it != path_cells.rend(); ++it) {
			result.push_back(pathfinding_node{ *it });
		}

		return result;
	}

	/*
		Create lambdas suitable for passing to algorithm functions.
		These capture this graph_view and forward to member functions.
	*/

	auto make_get_visited() {
		return [this](const vec2u c) { return get_visited(c); };
	}

	auto make_set_visited() {
		return [this](const vec2u c) { set_visited(c); };
	}

	auto make_set_parent() {
		return [this](const vec2u child, const vec2u parent) { set_parent(child, parent); };
	}

	auto make_get_g_cost() {
		return [this](const vec2u c) { return get_g_cost(c); };
	}

	auto make_set_g_cost() {
		return [this](const vec2u c, const float cost) { set_g_cost(c, cost); };
	}

	template <class Filter>
	auto make_for_each_neighbor(Filter&& filter) {
		return [this, filter = std::forward<Filter>(filter)](const vec2u c, auto&& callback) {
			this->for_each_neighbor(c, filter, callback);
		};
	}

	/*
		Iterate over 4-directional neighbors that are within bounds (no other filtering).
		Used for BFS that needs to traverse all cells regardless of walkability.
	*/
	auto make_for_each_neighbor_all() {
		return make_for_each_neighbor([this](const vec2u c) {
			return island.is_within_bounds(c);
		});
	}
};
