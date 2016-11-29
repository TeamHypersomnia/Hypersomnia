#pragma once
#include "game/components/transform_component.h"

struct camera_cone {
	components::transform transform;
	vec2 visible_world_area;

	// used for visibility queries
	ltrb get_transformed_visible_world_area_aabb() const {
		return augs::get_aabb_rotated(visible_world_area, transform.rotation) + transform.pos - visible_world_area / 2;
	}
};