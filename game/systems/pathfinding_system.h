#pragma once
class cosmos;

class pathfinding_system {
public:
	pathfinding_system();

	void advance_pathfinding_sessions(cosmos&);

	float epsilon_max_segment_difference;
	float epsilon_distance_visible_point;
	float epsilon_distance_the_same_vertex;

	int draw_memorised_walls;
	int draw_undiscovered;
};