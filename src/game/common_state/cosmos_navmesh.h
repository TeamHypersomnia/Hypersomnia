#pragma once
#include <vector>
#include "augs/math/vec2.h"
#include "game/common_state/cosmos_pathfinding.h"

/*
	Navmesh for arena_mode_ai.
	A simple square grid where cells are marked as free or occupied.
*/

struct navmesh_portal {
	// GEN INTROSPECTOR struct navmesh_portal
	vec2i in_cell_pos = vec2i::zero;
	vec2i out_cell_pos = vec2i::zero;
	int out_island_index = 0;
	// END GEN INTROSPECTOR
};

struct cosmos_navmesh_island {
	/*
		0  - free
		1  - occupied
		>2 - portals, identified (2 + portal_index) 
			 considered occupied, EXCEPT when targeting this exact portal.
	*/
	// GEN INTROSPECTOR struct cosmos_navmesh_island
	std::vector<uint8_t> occupied;
	std::vector<navmesh_portal> portals;

	ltrbi bound;
	int cell_size = 0;
	// END GEN INTROSPECTOR

	vec2i get_size_in_cells() const {
		if (cell_size <= 0) {
			return vec2i::zero;
		}

		return vec2i((bound.r - bound.l) / cell_size,
		(bound.b - bound.t) / cell_size);
	}

	void resize_to_bounds() {
		const auto size = get_size_in_cells();

		occupied.clear();
		occupied.resize(static_cast<std::size_t>(size.x * size.y), 0);
	}

	/*
		Convert cell position to linear index.
	*/

	std::size_t cell_index(const vec2i cell_pos) const {
		const auto size = get_size_in_cells();
		return static_cast<std::size_t>(cell_pos.y * size.x + cell_pos.x);
	}

	/*
		Grid-cell coordinate access (cell_pos is an index into the grid).
	*/

	uint8_t get_cell(const vec2i cell_pos) const {
		const auto size = get_size_in_cells();

		if (cell_pos.x < 0 || cell_pos.y < 0 || cell_pos.x >= size.x ||
		cell_pos.y >= size.y) {
			return 1;
		}

		const auto idx = cell_index(cell_pos);

		if (idx >= occupied.size()) {
			return 1;
		}

		return occupied[idx];
	}

	void set_cell(const vec2i cell_pos, const uint8_t value) {
		const auto size = get_size_in_cells();

		if (cell_pos.x < 0 || cell_pos.y < 0 || cell_pos.x >= size.x ||
		cell_pos.y >= size.y) {
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
		if (cell_size <= 0) {
			return 1;
		}

		const auto cell_pos = vec2i(
			(x - bound.l) / cell_size,
			(y - bound.t) / cell_size
		);

		return get_cell(cell_pos);
	}

	void set(const int x, const int y, const uint8_t value) {
		if (cell_size <= 0) {
			return;
		}

		const auto cell_pos = vec2i(
			(x - bound.l) / cell_size,
			(y - bound.t) / cell_size
		);

		set_cell(cell_pos, value);
	}
};

struct cosmos_navmesh {
	// GEN INTROSPECTOR struct cosmos_navmesh
	std::vector<cosmos_navmesh_island> islands;
	// END GEN INTROSPECTOR
};
