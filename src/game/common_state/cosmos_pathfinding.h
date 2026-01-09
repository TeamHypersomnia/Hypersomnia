#pragma once
#include <optional>
#include <queue>
#include <vector>
#include "augs/math/vec2.h"
#include "augs/algorithm/a_star.hpp"
#include "augs/algorithm/bfs.hpp"

/*
	Pathfinding structures.
*/

/*
	Type alias for island identification.
	Use this instead of std::size_t for island indices.
*/

using island_id_type = uint32_t;

struct pathfinding_node {
	// GEN INTROSPECTOR struct pathfinding_node
	vec2u cell_xy = vec2u::zero;
	// END GEN INTROSPECTOR
};

struct cell_on_island {
	// GEN INTROSPECTOR struct cell_on_island
	island_id_type island_index = 0;
	pathfinding_node node;
	// END GEN INTROSPECTOR
};

struct pathfinding_path {
	// GEN INTROSPECTOR struct pathfinding_path
	island_id_type island_index = 0;
	std::vector<pathfinding_node> nodes;
	std::optional<cell_on_island> final_portal_node;
	// END GEN INTROSPECTOR
};

/*
	Progress through a single pathfinding_path.
	Consolidates path + current node index into one struct.
	
	Use with std::optional<pathfinding_progress> to determine if pathfinding is active.
*/

struct pathfinding_progress {
	// GEN INTROSPECTOR struct pathfinding_progress
	pathfinding_path path;
	uint32_t node_index = 0;
	// END GEN INTROSPECTOR
};

/*
	Context for reusing allocations across pathfinding sessions.
*/

struct pathfinding_context {
	std::vector<uint8_t> islands_pathfinding_visited;
	std::vector<uint8_t> cells_pathfinding_visited;

	/*
		Reusable allocations for A* within-island pathfinding.
	*/
	std::vector<int> cells_parent;
	std::vector<float> cells_g_costs;

	/*
		Reusable queues for pathfinding algorithms.
	*/
	augs::astar_queue_type<vec2u> astar_queue;
	augs::bfs_queue_type<island_id_type> bfs_queue;
};

/*
	4-directional cell navigation constants.
*/

inline constexpr vec2i CELL_DIRECTIONS[4] = {
	{ 0, -1 },
	{ 0,  1 },
	{ -1, 0 },
	{ 1,  0 }
};

/*
	Maximum number of candidate cells to consider when finding closest walkable cell.
	Abort BFS after finding this many matches to avoid traversing entire graph.
*/

inline constexpr std::size_t MAX_WALKABLE_CANDIDATES = 4;
