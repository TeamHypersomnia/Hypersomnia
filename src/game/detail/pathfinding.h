#pragma once
#include <optional>
#include <vector>
#include "game/common_state/cosmos_navmesh.h"
#include "augs/enums/callback_result.h"

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

std::optional<std::size_t> find_island_for_position(const cosmos_navmesh& navmesh, const vec2 world_pos);

/*
	BFS on islands to find path from source island to target island.
	Returns the next island index to go to (std::nullopt if no path found).
	If source == target, returns source.
*/

std::optional<std::size_t> find_islands_connection(
	const cosmos_navmesh& navmesh,
	const std::size_t source_island_index,
	const std::size_t target_island_index,
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
	const std::size_t source_island_index,
	const std::size_t target_island_index,
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
	const std::size_t source_island_index,
	const std::size_t target_island_index,
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

/*
	Fat Line of Sight (FLoS) check.
	Uses a rectangle query instead of a raycast to ensure nothing stands in the character's way.
	The rectangle's short ends touch the source and destination, with a configurable width.
	Uses predefined_queries::pathfinding() filter.
*/

class physics_world_cache;
class si_scaling;

bool fat_line_of_sight(
	const physics_world_cache& physics,
	const si_scaling& si,
	const vec2 source_pos,
	const vec2 target_pos,
	const float width,
	const entity_id ignore_entity = entity_id()
);

/*
	BFS to find the closest unoccupied cell from a given cell that may be occupied.
	Returns the unoccupied cell closest to the world position (Euclidean distance).
	Used when pathfinding source is on an occupied cell.
*/

struct unoccupied_cell_result {
	vec2u cell;
	std::vector<pathfinding_node> path_through_occupied;
};

std::optional<unoccupied_cell_result> find_closest_unoccupied_cell(
	const cosmos_navmesh_island& island,
	const vec2u start_cell,
	const vec2 world_pos,
	const std::optional<std::size_t> target_portal_index = std::nullopt,
	pathfinding_context* ctx = nullptr
);

/*
	Random walkable cell within n steps using BFS with random direction choices.
	Uses stable_rng for deterministic random directions.
*/

class randomization;

std::optional<vec2u> find_random_walkable_cell_within_steps(
	const cosmos_navmesh_island& island,
	const vec2u start_cell,
	const uint32_t max_steps,
	randomization& rng,
	pathfinding_context* ctx = nullptr
);
