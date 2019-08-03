#pragma once

inline void vis_response_to_triangles(
	const visibility_response& response, 
	augs::vertex_triangle_buffer& triangles, 
	const rgba col,
	const vec2 eye_pos
) {
	const auto triangles_n = response.get_num_triangles();
	triangles.resize(triangles_n);

	for (std::size_t t = 0; t < triangles_n; ++t) {
		const auto world_light_tri = response.get_world_triangle(t, eye_pos);

		auto& renderable_light_tri = triangles[t];

		for (int i = 0; i < 3; ++i) {
			renderable_light_tri.vertices[i].pos = world_light_tri[i];
			renderable_light_tri.vertices[i].color = col;
		}
	}
};

