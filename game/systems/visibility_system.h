#pragma once
class cosmos;

class visibility_system {
public:
	void generate_visibility_and_sight_information(cosmos&);

	bool draw_triangle_edges = true;
	bool draw_cast_rays = false;
	bool draw_discontinuities = false;
	bool draw_visible_walls = false;

	float epsilon_ray_distance_variation = 0.f;
	float epsilon_distance_vertex_hit = 0.f;
	float epsilon_threshold_obstacle_hit = 0.f;
};