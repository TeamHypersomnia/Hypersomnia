#pragma once

/*
    Placeholder navmesh for arena_mode_ai.
    Will be expanded with actual data structures.
*/

struct cosmos_navmesh_island {
	// GEN INTROSPECTOR struct cosmos_navmesh_island
	std::vector<uint8_t> occupied;

	ltrbi bound;
	int cell_size = 0;
	// END GEN INTROSPECTOR
};

struct cosmos_navmesh {
	// GEN INTROSPECTOR struct cosmos_navmesh
	std::vector<cosmos_navmesh_island> islands;
	// END GEN INTROSPECTOR
};
