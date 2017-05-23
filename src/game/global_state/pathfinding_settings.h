#pragma once

struct pathfinding_settings {
	// GEN INTROSPECTOR struct pathfinding_settings
	float epsilon_distance_visible_point = 2.f;
	float epsilon_distance_the_same_vertex = 50.f;

	short draw_memorised_walls = false;
	short draw_undiscovered = false;
	// END GEN INTROSPECTOR
};