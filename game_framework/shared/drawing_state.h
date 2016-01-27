#pragma once
#include "math/vec2.h"
#include "math/rects.h"
#include "../components/transform_component.h"

#include "utilities/entity_system/entity_id.h"

namespace augs {
	class renderer;
}

namespace shared {
	struct drawing_state {
		augs::renderer* output = nullptr;

		components::transform object_transform;
		components::transform camera_transform;
		
		vec2 visible_world_area;

		// used for visibility queries
		augs::rects::ltrb<float> transformed_visible_world_area_aabb;

		augs::rects::xywh<int> viewport;

		augs::entity_id subject;

	};
}