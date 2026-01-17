#pragma once
#include <optional>
#include <vector>
#include "game/common_state/cosmos_navmesh.h"
#include "augs/enums/callback_result.h"
#include "augs/math/transform.h"

/*
	Pathfinding algorithms using A* for navmesh.
*/

/*
	Convert cell coordinates to world position (center of cell).
*/

vec2 cell_to_world(const cosmos_navmesh_island& island, const vec2u cell_xy);

/*
	Convert world position to cell coordinates.
*/

vec2u world_to_cell(const cosmos_navmesh_island& island, const vec2 world_pos);

/*
	Manhattan distance between two cells (for 4-directional A*).
*/

uint32_t cell_distance(const vec2u a, const vec2u b);

/*
	Euclidean distance between two world positions.
*/

float world_distance(const vec2 a, const vec2 b);

/*
	Find which island index contains a given world position.
	Returns std::nullopt if position is not within any island.
*/

std::optional<island_id_type> find_island_for_position(const cosmos_navmesh& navmesh, const vec2 world_pos);

/*
	BFS on islands to find path from source island to target island.
	Returns the next island index to go to (std::nullopt if no path found).
	If source == target, returns source.
*/

std::optional<island_id_type> find_islands_connection(
	const cosmos_navmesh& navmesh,
	const island_id_type source_island_index,
	const island_id_type target_island_index,
	pathfinding_context* ctx = nullptr
);

/*
	Find best portal from source island to target island.
	Returns portal index on source island that minimizes:
	  euclidean_distance(from_pos -> portal_center) + euclidean_distance(portal_exit -> to_pos)
	
	Returns std::nullopt if no suitable portal found.
*/

std::optional<std::size_t> find_best_portal_from_to(
	const cosmos_navmesh& navmesh,
	const island_id_type source_island_index,
	const island_id_type target_island_index,
	const vec2 from_pos,
	const vec2 to_pos
);

/*
	A* pathfinding within a single island.
	
	target_portal_index: if set, we're navigating to a specific portal.
	                     Its cells are walkable; other portal cells are blocked.
	                     If nullopt, we're navigating to a regular cell.

	Returns a vector of pathfinding nodes, or nullopt if no path found.
*/

std::optional<std::vector<pathfinding_node>> find_path_within_island(
	const cosmos_navmesh_island& island,
	const vec2u start_cell,
	const vec2u target_cell,
	const std::optional<std::size_t> target_portal_index = std::nullopt,
	pathfinding_context* ctx = nullptr
);

/*
	Find path across islands with direct connection.
	Uses find_best_portal_from_to to find the best portal.
*/

std::optional<pathfinding_path> find_path_across_islands_direct(
	const cosmos_navmesh& navmesh,
	const island_id_type source_island_index,
	const island_id_type target_island_index,
	const vec2 source_pos,
	const vec2 target_pos,
	pathfinding_context* ctx = nullptr
);

/*
	Find path across multiple islands (unknown number on path).
	First does BFS on islands, then uses find_path_across_islands_direct.
	Only returns path to the next portal (or directly to destination if same island).
*/

std::optional<pathfinding_path> find_path_across_islands_many(
	const cosmos_navmesh& navmesh,
	const vec2 source_pos,
	const vec2 target_pos,
	pathfinding_context* ctx = nullptr
);

/*
	Helper function that repeatedly calls the main pathfinding function
	until final_portal_exit is empty.
	Returns all paths concatenated for visualization.
*/

std::vector<pathfinding_path> find_path_across_islands_many_full(
	const cosmos_navmesh& navmesh,
	const vec2 source_pos,
	const vec2 target_pos,
	pathfinding_context* ctx = nullptr
);

/*
	BFS to find the closest unoccupied cell from a given cell that may be occupied.
	Returns the unoccupied cell closest to the world position (Euclidean distance).
	Used when pathfinding source is on an occupied cell.
	
	Only finds cells with value == 0 (unoccupied). Does not handle portals.
*/

struct unoccupied_cell_result {
	vec2u cell;
	std::vector<pathfinding_node> path_through_occupied;
};

std::optional<unoccupied_cell_result> find_closest_unoccupied_cell(
	const cosmos_navmesh_island& island,
	const vec2u start_cell,
	const vec2 world_pos,
	pathfinding_context* ctx = nullptr
);

/*
	Random unoccupied cell within n steps using random walk with random direction choices.
	Uses stable_rng for deterministic random directions.
	Only walks through unoccupied cells (value == 0), avoiding portals.
*/

struct randomization;

std::optional<vec2u> find_random_unoccupied_cell_within_steps(
	const cosmos_navmesh_island& island,
	const vec2u start_cell,
	const uint32_t max_steps,
	randomization& rng,
	pathfinding_context* ctx = nullptr
);

/*
	Overload that accepts world coordinates and navmesh.
	Finds the island for the position and delegates to the cell-based version.
*/

std::optional<vec2> find_random_unoccupied_position_within_steps(
	const cosmos_navmesh& navmesh,
	const vec2 world_pos,
	const uint32_t max_steps,
	randomization& rng,
	pathfinding_context* ctx = nullptr
);

/*
	Find a random unoccupied cell within a (potentially rotated) rectangle.
	Used for picking random points on bombsites.
	
	Steps:
	1) Take AABB of the zone
	2) Match the grid subsection
	3) For every unoccupied cell, check if point hovers the rotated rectangle
	4) Return random one of these cell centers as destination
*/

std::optional<vec2> find_random_unoccupied_cell_within_rect(
	const cosmos_navmesh& navmesh,
	const ltrb& aabb,
	const transformr& rect_transform,
	const vec2 rect_size,
	randomization& rng
);

/*
	Find pathfinding target for a bomb.
	Returns the closest walkable cell that the bomb touches.
	If no walkable cell is found, teleports the bomb to the closest walkable cell.
*/

struct bomb_pathfinding_target {
	vec2 target_position;
	bool bomb_was_teleported = false;
};

template <class E>
std::optional<bomb_pathfinding_target> find_bomb_pathfinding_target(
	const E& bomb_entity,
	const cosmos_navmesh& navmesh,
	const vec2 source_pos
);
