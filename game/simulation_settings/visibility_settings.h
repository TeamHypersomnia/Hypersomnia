#pragma once

struct visibility_settings {
	// GEN INTROSPECTOR struct visibility_settings
	bool draw_triangle_edges = false;
	bool draw_cast_rays = false;
	bool draw_discontinuities = false;
	bool draw_visible_walls = false;

	float epsilon_ray_distance_variation = 0.004f;
	float epsilon_distance_vertex_hit = 1.f;
	float epsilon_threshold_obstacle_hit = 10.f;
	// END GEN INTROSPECTOR
};
