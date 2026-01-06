#pragma once
#include <optional>
#include <vector>
#include "augs/math/vec2.h"

/*
	Pathfinding structures.
*/

struct pathfinding_node {
	// GEN INTROSPECTOR struct pathfinding_node
	vec2i cell_xy = vec2i::zero;
	// END GEN INTROSPECTOR
};

struct cell_on_island {
	// GEN INTROSPECTOR struct cell_on_island
	int island_index = 0;
	pathfinding_node node;
	// END GEN INTROSPECTOR
};

struct pathfinding_path {
	// GEN INTROSPECTOR struct pathfinding_path
	int island_index = 0;
	std::vector<pathfinding_node> nodes;
	std::optional<cell_on_island> final_portal_node;
	// END GEN INTROSPECTOR
};

/*
	Context for reusing allocations across pathfinding sessions.
*/

struct pathfinding_context {
	std::vector<uint8_t> islands_pathfinding_visited;
	std::vector<uint8_t> cells_pathfinding_visited;
};
