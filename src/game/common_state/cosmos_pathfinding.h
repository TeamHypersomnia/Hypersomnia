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

	pathfinding_node() = default;
	pathfinding_node(const vec2u xy) : cell_xy(xy) {}

	bool operator==(const pathfinding_node& other) const = default;
};

struct cell_on_navmesh {
	cell_on_navmesh() = default;
	cell_on_navmesh(const island_id_type i, const vec2u v) : island_index(i), node(v) {}

	// GEN INTROSPECTOR struct cell_on_navmesh
	island_id_type island_index = 0;
	pathfinding_node node;
	// END GEN INTROSPECTOR

	bool operator==(const cell_on_navmesh& other) const = default;
};

struct pathfinding_path {
	// GEN INTROSPECTOR struct pathfinding_path
	island_id_type island_index = 0;
	std::vector<pathfinding_node> nodes;
	std::optional<cell_on_navmesh> final_portal_exit;
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
	8-directional cell navigation constants for diagonal pathfinding.
	First 4 are cardinal directions (weight 1.0), last 4 are diagonal (weight sqrt(2)).
*/

inline constexpr float SQRT_2 = 1.41421356237f;

inline constexpr vec2i CELL_DIRECTIONS_8[8] = {
	/* Cardinal directions */
	{ 0, -1 },   /* up */
	{ 0,  1 },   /* down */
	{ -1, 0 },   /* left */
	{ 1,  0 },   /* right */
	/* Diagonal directions */
	{ -1, -1 },  /* top-left */
	{ 1, -1 },   /* top-right */
	{ -1, 1 },   /* bottom-left */
	{ 1,  1 }    /* bottom-right */
};

inline constexpr float CELL_WEIGHTS_8[8] = {
	/* Cardinal directions */
	1.0f,
	1.0f,
	1.0f,
	1.0f,
	/* Diagonal directions */
	SQRT_2,
	SQRT_2,
	SQRT_2,
	SQRT_2
};

/*
	Maximum number of candidate cells to consider when finding closest walkable cell.
	Abort BFS after finding this many matches to avoid traversing entire graph.
*/

inline constexpr std::size_t MAX_WALKABLE_CANDIDATES = 4;
