#pragma once
#include <optional>
#include <vector>
#include "game/common_state/cosmos_navmesh.h"

/*
	Pathfinding algorithms using A* for navmesh.
*/

/*
	Convert cell coordinates to world position (center of cell).
*/

vec2 cell_to_world(const cosmos_navmesh_island& island, const vec2i cell_xy);

/*
	Convert world position to cell coordinates.
*/

vec2i world_to_cell(const cosmos_navmesh_island& island, const vec2 world_pos);

/*
	Manhattan distance between two cells (for 4-directional A*).
*/

int cell_distance(const vec2i a, const vec2i b);

/*
	Euclidean distance between two world positions.
*/

float world_distance(const vec2 a, const vec2 b);

/*
	Find which island index contains a given world position.
	Returns std::nullopt if position is not within any island.
*/

std::optional<int> find_island_for_position(const cosmos_navmesh& navmesh, const vec2 world_pos);

/*
	BFS on islands to find path from source island to target island.
	Returns the next island index to go to (std::nullopt if no path found).
	If source == target, returns source.
*/

std::optional<int> find_islands_connection(
	const cosmos_navmesh& navmesh,
	const int source_island_index,
	const int target_island_index,
	pathfinding_context* ctx = nullptr
);

/*
	Find best portal from source island to target island.
	Returns portal index on source island that minimizes:
	  euclidean_distance(from_pos -> portal_center) + euclidean_distance(portal_exit -> to_pos)
	
	Returns std::nullopt if no suitable portal found.
*/

std::optional<int> find_best_portal_from_to(
	const cosmos_navmesh& navmesh,
	const int source_island_index,
	const int target_island_index,
	const vec2 from_pos,
	const vec2 to_pos
);

/*
	A* pathfinding within a single island.
	
	target_portal_index: if >= 0, we're navigating to a specific portal.
	                     Its cells are walkable; other portal cells are blocked.
	                     If -1, we're navigating to a regular cell.

	Returns empty optional if no path found.
*/

std::optional<pathfinding_path> find_path_within_island(
	const cosmos_navmesh& navmesh,
	const int island_index,
	const vec2i start_cell,
	const vec2i target_cell,
	const int target_portal_index = -1,
	pathfinding_context* ctx = nullptr
);

/*
	Find path across islands with direct connection.
	Uses find_best_portal_from_to to find the best portal.
*/

std::optional<pathfinding_path> find_path_across_islands_direct(
	const cosmos_navmesh& navmesh,
	const int source_island_index,
	const int target_island_index,
	const vec2 source_pos,
	const vec2 target_pos,
	pathfinding_context* ctx = nullptr
);

/*
	Find path across multiple islands (unknown number on path).
	First does BFS on islands, then uses find_path_across_islands_direct.
*/

std::optional<pathfinding_path> find_path_across_islands_many(
	const cosmos_navmesh& navmesh,
	const vec2 source_pos,
	const vec2 target_pos,
	pathfinding_context* ctx = nullptr
);

/*
	Helper function that repeatedly calls the main pathfinding function
	until final_portal_node is empty.
	Returns all paths concatenated for visualization.
*/

std::vector<pathfinding_path> find_path_across_islands_many_full(
	const cosmos_navmesh& navmesh,
	const vec2 source_pos,
	const vec2 target_pos,
	pathfinding_context* ctx = nullptr
);
