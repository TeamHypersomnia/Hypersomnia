#include "movement_component.h"

void movement_flags::set_from_closest_direction(vec2 d) {
	left = right = forward = backward = false;

	static const vec2 dirs[] = {
		vec2::from_degrees(0),
		vec2::from_degrees(45 * 1),
		vec2::from_degrees(45 * 2),
		vec2::from_degrees(45 * 3),
		vec2::from_degrees(45 * 4),
		vec2::from_degrees(45 * 5),
		vec2::from_degrees(45 * 6),
		vec2::from_degrees(45 * 7)
	};

	const auto dir_num = std::min_element(dirs, dirs + 8, [d](vec2 a, vec2 b) { return a.dot(d) > b.dot(d); }) - dirs;

	if (dir_num == 0) {
		right = true;
	}
	else if (dir_num == 1) {
		right = true;
		backward = true;
	}
	else if (dir_num == 2) {
		backward = true;
	}
	else if (dir_num == 3) {
		left = true;
		backward = true;
	}
	else if (dir_num == 4) {
		left = true;
	}
	else if (dir_num == 5) {
		left = true;
		forward = true;
	}
	else if (dir_num == 6) {
		forward = true;
	}
	else if (dir_num == 7) {
		right = true;
		forward = true;
	}
}

void movement_flags::set_flags_from_target_direction(const vec2 d) {
	left = right = forward = backward = false;
	if (d.x > 0) right = true;
	if (d.y > 0) backward = true;
	if (d.x < 0) left = true;
	if (d.y < 0) forward = true;
}

vec2 movement_flags::get_force_requested_by_input(const vec2& axes) const {
	return {
		right * axes.x - left * axes.x,
		backward * axes.y - forward * axes.y
	};
}
	
namespace components {
	void movement::reset_movement_flags() {
		flags = {};
	}
}