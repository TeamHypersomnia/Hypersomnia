#pragma once
#include <vector>
#include "augs/math/vec2.h"

/*
	Navmesh for arena_mode_ai.
	A simple square grid where cells are marked as free or occupied.
*/

struct cosmos_navmesh_island {
	// GEN INTROSPECTOR struct cosmos_navmesh_island
	std::vector<uint8_t> occupied;

	ltrbi bound;
	int cell_size = 0;
	// END GEN INTROSPECTOR

	int get_width_in_cells() const {
		if (cell_size <= 0) {
			return 0;
		}

		return (bound.r - bound.l) / cell_size;
	}

	int get_height_in_cells() const {
		if (cell_size <= 0) {
			return 0;
		}

		return (bound.b - bound.t) / cell_size;
	}

	void resize_to_bounds() {
		const auto w = get_width_in_cells();
		const auto h = get_height_in_cells();

		occupied.clear();
		occupied.resize(static_cast<std::size_t>(w * h), 0);
	}

	/*
		Grid-cell coordinate access (cell_x, cell_y are indices into the grid).
	*/

	uint8_t get_cell(const int cell_x, const int cell_y) const {
		const auto w = get_width_in_cells();
		const auto h = get_height_in_cells();

		if (cell_x < 0 || cell_y < 0 || cell_x >= w || cell_y >= h) {
			return 1;
		}

		const auto idx = static_cast<std::size_t>(cell_y * w + cell_x);

		if (idx >= occupied.size()) {
			return 1;
		}

		return occupied[idx];
	}

	void set_cell(const int cell_x, const int cell_y, const uint8_t value) {
		const auto w = get_width_in_cells();
		const auto h = get_height_in_cells();

		if (cell_x < 0 || cell_y < 0 || cell_x >= w || cell_y >= h) {
			return;
		}

		const auto idx = static_cast<std::size_t>(cell_y * w + cell_x);

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

		const auto cell_x = (x - bound.l) / cell_size;
		const auto cell_y = (y - bound.t) / cell_size;

		return get_cell(cell_x, cell_y);
	}

	void set(const int x, const int y, const uint8_t value) {
		if (cell_size <= 0) {
			return;
		}

		const auto cell_x = (x - bound.l) / cell_size;
		const auto cell_y = (y - bound.t) / cell_size;

		set_cell(cell_x, cell_y, value);
	}
};

struct cosmos_navmesh {
	// GEN INTROSPECTOR struct cosmos_navmesh
	std::vector<cosmos_navmesh_island> islands;
	// END GEN INTROSPECTOR
};
