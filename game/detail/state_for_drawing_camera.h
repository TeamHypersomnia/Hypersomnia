#pragma once
#include "math/vec2.h"
#include "math/rects.h"
#include "game/components/transform_component.h"

struct state_for_drawing_camera {
	entity_id associated_character;
	components::transform camera_transform;

	vec2 visible_world_area;

	// used for visibility queries
	augs::rects::ltrb<float> transformed_visible_world_area_aabb;
};