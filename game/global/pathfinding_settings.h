#pragma once

struct pathfinding_settings {
	float epsilon_distance_visible_point = 2.f;
	float epsilon_distance_the_same_vertex = 50.f;

	short draw_memorised_walls = false;
	short draw_undiscovered = false;

	template <class Archive>
	void serialize(Archive& ar) {
		ar(
			CEREAL_NVP(epsilon_distance_visible_point),
			CEREAL_NVP(epsilon_distance_the_same_vertex),

			CEREAL_NVP(draw_memorised_walls),
			CEREAL_NVP(draw_undiscovered)
		);
	}
};