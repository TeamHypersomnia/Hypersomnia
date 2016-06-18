#pragma once
#include "math/vec2.h"
#include "math/rects.h"
#include "game/components/transform_component.h"

#include "game/entity_id.h"
#include "augs/graphics/vertex.h"

namespace augs {
	class renderer;
}

namespace shared {
	struct state_for_drawing_camera {
		augs::renderer* output = nullptr;

		components::transform camera_transform;

		vec2 visible_world_area;

		// used for visibility queries
		augs::rects::ltrb<float> transformed_visible_world_area_aabb;
		augs::rects::xywh<int> viewport;
	};

	struct state_for_drawing_renderable : state_for_drawing_camera {
		std::vector<augs::vertex_triangle>* overridden_target_buffer = nullptr;

		components::transform renderable_transform;
		entity_id renderable;
		bool screen_space_mode = false;
		bool position_is_left_top_corner = false;

		augs::rgba colorize = augs::white;

		void setup_camera_state(state_for_drawing_camera b) {
			state_for_drawing_camera::operator=(b);
		}
	};
}