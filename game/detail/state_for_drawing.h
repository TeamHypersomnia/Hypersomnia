#pragma once
#include "math/vec2.h"
#include "math/rects.h"
#include "game/components/transform_component.h"

#include "game/entity_id.h"
#include "game/entity_handle.h"
#include "augs/graphics/vertex.h"

namespace augs {
	class renderer;
}

struct with_target_buffer {
	std::vector<augs::vertex_triangle>& target_buffer;
	with_target_buffer(std::vector<augs::vertex_triangle>&);
};

struct state_for_drawing_camera {
	components::transform camera_transform;

	vec2 visible_world_area;

	// used for visibility queries
	augs::rects::ltrb<float> transformed_visible_world_area_aabb;
};