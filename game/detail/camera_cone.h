#pragma once
#include "game/components/transform_component.h"

struct camera_cone {
	components::transform transform;
	vec2 visible_world_area;

	vec2 operator[](const vec2 pos) const;
	vec2 get_screen_space_0_1(const vec2 pos) const;
	vec2 get_screen_space_revert_y(const vec2 pos) const;
	ltrb get_transformed_visible_world_area_aabb() const;
};