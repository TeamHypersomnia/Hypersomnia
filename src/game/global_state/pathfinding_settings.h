#pragma once
#include "augs/padding_byte.h"

struct pathfinding_settings {
	// GEN INTROSPECTOR struct pathfinding_settings
	float epsilon_distance_visible_point = 2.f;
	float epsilon_distance_the_same_vertex = 50.f;

	bool draw_memorised_walls = false;
	bool draw_undiscovered = false;
	pad_bytes<2> pad;
	// END GEN INTROSPECTOR
};