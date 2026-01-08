#pragma once
#include <vector>
#include "augs/math/vec2.h"
#include "game/common_state/cosmos_pathfinding.h"

/*
	Navmesh for arena_mode_ai.
	A simple square grid where cells are marked as walkable or occupied.

	Cell value meanings:
		0  - cell is *walkable* and *unoccupied*
		1  - cell is *unwalkable* and *occupied*
		>=2 - cell is *walkable* but *occupied* (portals, identified as 2 + portal_index)
*/

/*
	Global helper functions for cell type checks (for use outside island context).
*/

inline bool is_cell_walkable(const uint8_t cell_value) {
	return cell_value != 1;
}

inline bool is_cell_unoccupied(const uint8_t cell_value) {
	return cell_value == 0;
}

inline bool is_cell_occupied(const uint8_t cell_value) {
	return cell_value != 0;
}

inline bool is_cell_portal(const uint8_t cell_value) {
	return cell_value >= 2;
}

inline bool is_cell_unwalkable(const uint8_t cell_value) {
	return cell_value == 1;
}

/*
	Check if a cell value matches a specific portal index.
	If target_portal_index is nullopt, returns false.
*/

inline bool is_cell_target_portal(const uint8_t cell_value, const std::optional<std::size_t> target_portal_index) {
	if (!target_portal_index.has_value()) {
		return false;
	}
	return cell_value == static_cast<uint8_t>(2 + *target_portal_index);
}

struct navmesh_portal {
	// GEN INTROSPECTOR struct navmesh_portal
	vec2u in_cell_pos = vec2u::zero;
	vec2u out_cell_pos = vec2u::zero;
	int out_island_index = 0;
	// END GEN INTROSPECTOR
};

struct cosmos_navmesh_island {
	/*
		Cell values - see helper functions above:
			0  - walkable and unoccupied
			1  - unwalkable and occupied
			>=2 - walkable but occupied (portals, 2 + portal_index)
	*/
	// GEN INTROSPECTOR struct cosmos_navmesh_island
	std::vector<uint8_t> occupied;
	std::vector<navmesh_portal> portals;

	ltrbi bound;
	uint32_t cell_size = 0;
	// END GEN INTROSPECTOR

	vec2u get_size_in_cells() const {
		if (cell_size == 0) {
			return vec2u::zero;
		}

		return vec2u(
			static_cast<uint32_t>(bound.w()) / cell_size,
			static_cast<uint32_t>(bound.h()) / cell_size
		);
	}

	void resize_to_bounds() {
		const auto size = get_size_in_cells();

		occupied.clear();
		occupied.resize(size.area(), 0);
	}

	/*
		Convert cell position to linear index.
	*/

	std::size_t cell_index(const vec2u cell_pos) const {
		const auto size = get_size_in_cells();
		return static_cast<std::size_t>(cell_pos.y * size.x + cell_pos.x);
	}

	/*
		Grid-cell coordinate access (cell_pos is an index into the grid).
	*/

	uint8_t get_cell(const vec2u cell_pos) const {
		const auto size = get_size_in_cells();

		if (cell_pos.x >= size.x || cell_pos.y >= size.y) {
			return 1;
		}

		const auto idx = cell_index(cell_pos);

		if (idx >= occupied.size()) {
			return 1;
		}

		return occupied[idx];
	}

	void set_cell(const vec2u cell_pos, const uint8_t value) {
		const auto size = get_size_in_cells();

		if (cell_pos.x >= size.x || cell_pos.y >= size.y) {
			return;
		}

		const auto idx = cell_index(cell_pos);

		if (idx >= occupied.size()) {
			return;
		}

		occupied[idx] = value;
	}

	/*
		World-coordinate access (x, y are in pixels/world units).
	*/

	uint8_t get(const int x, const int y) const {
		if (cell_size == 0) {
			return 1;
		}

		if (x < bound.l || y < bound.t) {
			return 1;
		}

		const auto cell_pos = vec2u(vec2i(x, y) - bound.lt()) / cell_size;
		return get_cell(cell_pos);
	}

	void set(const int x, const int y, const uint8_t value) {
		if (cell_size == 0) {
			return;
		}

		if (x < bound.l || y < bound.t) {
			return;
		}

		const auto cell_pos = vec2u(vec2i(x, y) - bound.lt()) / cell_size;
		set_cell(cell_pos, value);
	}

	/*
		Cell type check methods with bounds checking.
		These check if cell_pos is within bounds before checking cell type.
	*/

	bool is_cell_walkable(const vec2u cell_pos) const {
		return ::is_cell_walkable(get_cell(cell_pos));
	}

	bool is_cell_unoccupied(const vec2u cell_pos) const {
		const auto size = get_size_in_cells();

		if (cell_pos.x >= size.x || cell_pos.y >= size.y) {
			return false;
		}

		return ::is_cell_unoccupied(get_cell(cell_pos));
	}

	bool is_cell_occupied(const vec2u cell_pos) const {
		return ::is_cell_occupied(get_cell(cell_pos));
	}

	bool is_cell_portal(const vec2u cell_pos) const {
		return ::is_cell_portal(get_cell(cell_pos));
	}

	bool is_cell_unwalkable(const vec2u cell_pos) const {
		return ::is_cell_unwalkable(get_cell(cell_pos));
	}
};

struct cosmos_navmesh {
	// GEN INTROSPECTOR struct cosmos_navmesh
	std::vector<cosmos_navmesh_island> islands;
	// END GEN INTROSPECTOR
};
