#pragma once
#include "game/cosmos/entity_handle_declaration.h"
#include "augs/graphics/vertex.h"

#include "augs/math/camera_cone.h"
#include "game/detail/render_layer_filter.h"

class interpolation_system;

struct render_layer_filter;

struct aabb_highlighter_drawing_input {
	const augs::drawer_with_default output;
	const const_entity_handle& subject;
	const interpolation_system& interp;
	const camera_eye camera;
	const vec2 screen_size;
};

struct aabb_highlighter {
	float timer = 0.f;
	float cycle_duration_ms = 400.f;

	float base_gap = 2.f;
	float smallest_length = 8.f;
	float biggest_length = 16.f;
	float scale_down_when_aabb_no_bigger_than = 40.f;

	static render_layer_filter get_filter();
	static bool is_hoverable(const const_entity_handle);

	void update(const augs::delta);
	void draw(const aabb_highlighter_drawing_input) const;
};