#pragma once
#include "augs/graphics/vertex.h"
#include "game/enums/renderable_drawing_type.h"
#include "game/components/transform_component.h"
#include "game/detail/camera_cone.h"

struct basic_renderable_drawing_input : vertex_triangle_buffer_reference {
	using vertex_triangle_buffer_reference::vertex_triangle_buffer_reference;

	components::transform renderable_transform;
	camera_cone camera;

	rgba colorize = white;
	renderable_drawing_type drawing_type = renderable_drawing_type::NORMAL;

	void set_global_time_seconds(const double) {

	}
};
