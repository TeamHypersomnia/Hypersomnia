#pragma once

struct visibility_settings {
	bool draw_triangle_edges = true;
	bool draw_cast_rays = false;
	bool draw_discontinuities = false;
	bool draw_visible_walls = false;

	float epsilon_ray_distance_variation = 0.f;
	float epsilon_distance_vertex_hit = 0.f;
	float epsilon_threshold_obstacle_hit = 0.f;

	template <class Archive>
	void serialize(Archive& ar) {
		ar(
			CEREAL_NVP(draw_triangle_edges),
			CEREAL_NVP(draw_cast_rays),
			CEREAL_NVP(draw_discontinuities),
			CEREAL_NVP(draw_visible_walls),

			CEREAL_NVP(epsilon_ray_distance_variation),
			CEREAL_NVP(epsilon_distance_vertex_hit),
			CEREAL_NVP(epsilon_threshold_obstacle_hit)
		);
	}
};
