#pragma once

struct pathfinding_settings {
	float epsilon_distance_visible_point = 2.f;
	float epsilon_distance_the_same_vertex = 50.f;

	int draw_memorised_walls = false;
	int draw_undiscovered = false;
};