#include "movement_component.h"

namespace components {
	void movement::set_flags_from_closest_direction(vec2 d) {
		moving_left = moving_right = moving_forward = moving_backward = false;

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
			moving_right = true;
		}
		else if (dir_num == 1) {
			moving_right = true;
			moving_backward = true;
		}
		else if (dir_num == 2) {
			moving_backward = true;
		}
		else if (dir_num == 3) {
			moving_left = true;
			moving_backward = true;
		}
		else if (dir_num == 4) {
			moving_left = true;
		}
		else if (dir_num == 5) {
			moving_left = true;
			moving_forward = true;
		}
		else if (dir_num == 6) {
			moving_forward = true;
		}
		else if (dir_num == 7) {
			moving_right = true;
			moving_forward = true;
		}
	}

	void movement::set_flags_from_target_direction(const vec2 d) {
		moving_left = moving_right = moving_forward = moving_backward = false;
		if (d.x > 0) moving_right = true;
		if (d.y > 0) moving_backward = true;
		if (d.x < 0) moving_left = true;
		if (d.y < 0) moving_forward = true;
	}

	vec2 movement::get_force_requested_by_input(const vec2& axes) const {
		return {
			moving_right * axes.x - moving_left * axes.x,
			moving_backward * axes.y - moving_forward * axes.y
		};
	}
	
	void movement::reset_movement_flags() {
		moving_left = moving_right = moving_forward = moving_backward = walking_enabled = sprint_enabled = false;
	}
}