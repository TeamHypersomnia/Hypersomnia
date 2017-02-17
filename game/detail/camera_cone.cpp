#include "camera_cone.h"

vec2 camera_cone::operator[](const vec2 pos) const {
	return pos - transform.pos + visible_world_area / 2;
}

vec2 camera_cone::get_screen_space_0_1(const vec2 pos) const {
	return operator[](pos) / visible_world_area;
}

vec2 camera_cone::get_screen_space_revert_y(const vec2 pos) const {
	auto ss = operator[](pos);
	ss.y = visible_world_area.y - ss.y;
	return ss;
}

ltrb camera_cone::get_transformed_visible_world_area_aabb() const {
	return augs::get_aabb_rotated(visible_world_area, transform.rotation) + transform.pos - visible_world_area / 2;
}