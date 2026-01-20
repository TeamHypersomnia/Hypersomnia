#pragma once
#include "game/detail/pathfinding.h"
#include "game/cosmos/entity_handle.h"
#include "game/components/rigid_body_component.h"

/*
	Implementation of find_bomb_pathfinding_target.
	Find the closest walkable cell that the bomb touches.
*/

template <class E>
std::optional<bomb_pathfinding_target> find_bomb_pathfinding_target(
	const E& bomb_entity,
	const cosmos_navmesh& navmesh,
	const vec2 source_pos
) {
	if (!bomb_entity.alive()) {
		return std::nullopt;
	}

	(void)source_pos;

	const auto bomb_transform = bomb_entity.get_logic_transform();
	const auto bomb_pos = bomb_transform.pos;

	/*
		Find which island the bomb is on.
	*/
	const auto island_opt = ::find_island_for_position(navmesh, bomb_pos);

	if (!island_opt.has_value()) {
		return std::nullopt;
	}

	const auto island_idx = *island_opt;
	const auto& island = navmesh.islands[island_idx];
	const auto cell_size = static_cast<float>(island.cell_size);

	if (cell_size <= 0.0f) {
		return std::nullopt;
	}

	/*
		Get bomb AABB to determine which cells it touches.
	*/
	auto get_bomb_aabb = [&]() -> ltrb {
		if (const auto aabb = bomb_entity.find_aabb()) {
			return *aabb;
		}

		/*
			Fallback: estimate from position with a small radius.
		*/
		const auto fallback_size = 20.0f;
		return ltrb::center_and_size(bomb_pos, vec2::square(fallback_size));
	};

	const auto bomb_aabb = get_bomb_aabb();
	const auto size = island.get_size_in_cells();

	/*
		Find all cells that the bomb AABB intersects.
	*/
	const auto min_cell = ::world_to_cell(island, vec2(bomb_aabb.l, bomb_aabb.t));
	const auto max_cell = ::world_to_cell(island, vec2(bomb_aabb.r, bomb_aabb.b));

	std::vector<vec2u> walkable_cells;

	for (uint32_t y = min_cell.y; y <= max_cell.y && y < size.y; ++y) {
		for (uint32_t x = min_cell.x; x <= max_cell.x && x < size.x; ++x) {
			const auto cell = vec2u(x, y);
			const auto cell_value = island.get_cell(cell);

			/*
				Only consider unoccupied cells (value 0).
			*/
			if (::is_cell_unoccupied(cell_value)) {
				walkable_cells.push_back(cell);
			}
		}
	}

	if (!walkable_cells.empty()) {
		/*
			Find the walkable cell closest to source_pos.
		*/
		vec2u best_cell = walkable_cells[0];
		float best_dist_sq = (::cell_to_world(island, best_cell) - bomb_pos).length_sq();

		for (std::size_t i = 1; i < walkable_cells.size(); ++i) {
			const auto cell_world = ::cell_to_world(island, walkable_cells[i]);
			const auto dist_sq = (cell_world - bomb_pos).length_sq();

			if (dist_sq < best_dist_sq) {
				best_dist_sq = dist_sq;
				best_cell = walkable_cells[i];
			}
		}

		bomb_pathfinding_target result;
		result.set(bomb_pos, ::cell_to_world(island, best_cell));
		result.resolved_cell = cell_on_navmesh(island_idx, best_cell);
		result.bomb_was_teleported = false;
		return result;
	}

	/*
		Fallback: bomb touches no walkable tiles.
		BFS from bomb center to find closest unoccupied tile.
	*/
	const auto bomb_cell = ::world_to_cell(island, bomb_pos);
	const auto unoccupied_opt = ::find_closest_unoccupied_cell(island, bomb_cell, bomb_pos, nullptr);

	if (!unoccupied_opt.has_value()) {
		return std::nullopt;
	}

	const auto closest_unoccupied_world = ::cell_to_world(island, unoccupied_opt->cell);

	/*
		Teleport bomb to the closest unoccupied cell center.
		Note: This modifies the bomb entity position.
		The caller should handle this appropriately.
	*/
	bomb_pathfinding_target result;
	result.set(bomb_pos, closest_unoccupied_world);
	result.resolved_cell = cell_on_navmesh(island_idx, unoccupied_opt->cell);
	result.bomb_was_teleported = true;
	return result;
}
