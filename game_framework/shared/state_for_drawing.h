#pragma once
#include "math/vec2.h"
#include "math/rects.h"
#include "../components/transform_component.h"

#include "augs/entity_system/entity_id.h"

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
		components::transform renderable_transform;
		augs::entity_id renderable;
		bool screen_space_mode = false;

		void setup_camera_state(state_for_drawing_camera b) {
			state_for_drawing_camera::operator=(b);
		}
	};
}