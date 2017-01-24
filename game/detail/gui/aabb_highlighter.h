#pragma once
#include "game/transcendental/entity_handle_declaration.h"
#include "augs/graphics/vertex.h"

struct camera_cone;
class interpolation_system;

struct aabb_highlighter {
	float timer = 0.f;
	float cycle_duration_ms = 400.f;

	float base_gap = 2.f;
	float smallest_length = 8.f;
	float biggest_length = 16.f;
	float scale_down_when_aabb_no_bigger_than = 40.f;

	void update(const float delta_ms);

	void draw(
		augs::vertex_triangle_buffer& output,
		const const_entity_handle subject,
		const interpolation_system& interp,
		const camera_cone camera
	) const;
};