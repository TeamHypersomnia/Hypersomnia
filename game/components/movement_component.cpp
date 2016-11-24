#include "movement_component.h"

namespace components {
	void movement::set_flags_from_closest_direction(vec2 d) {
		moving_left = moving_right = moving_forward = moving_backward = false;

		static const vec2 dirs[] = {
			vec2().set_from_degrees(0),
			vec2().set_from_degrees(45 * 1),
			vec2().set_from_degrees(45 * 2),
			vec2().set_from_degrees(45 * 3),
			vec2().set_from_degrees(45 * 4),
			vec2().set_from_degrees(45 * 5),
			vec2().set_from_degrees(45 * 6),
			vec2().set_from_degrees(45 * 7)
		};

		const auto dir_num = std::min_element(dirs, dirs + 8, [d](vec2 a, vec2 b) { return a.radians_between(d) < b.radians_between(d); }) - dirs;

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

	void movement::set_flags_from_target_direction(vec2 d) {
		moving_left = moving_right = moving_forward = moving_backward = false;
		if (d.x > 0) moving_right = true;
		if (d.y > 0) moving_backward = true;
		if (d.x < 0) moving_left = true;
		if (d.y < 0) moving_forward = true;
	}

	void movement::add_animation_receiver(entity_id e, bool stop_at_zero_movement) {
		subscribtion s;
		s.target = e;
		s.stop_response_at_zero_speed = stop_at_zero_movement;
		response_receivers.push_back(s);
	}

	void movement::reset_movement_flags() {
		moving_left = moving_right = moving_forward = moving_backward = walking_enabled = sprint_enabled = false;
	}
}