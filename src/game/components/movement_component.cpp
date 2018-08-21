#include "movement_component.h"

namespace components {
	void movement::set_flags_from_closest_direction(vec2 d) {
		flags.left = flags.right = flags.forward = flags.backward = false;

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
			flags.right = true;
		}
		else if (dir_num == 1) {
			flags.right = true;
			flags.backward = true;
		}
		else if (dir_num == 2) {
			flags.backward = true;
		}
		else if (dir_num == 3) {
			flags.left = true;
			flags.backward = true;
		}
		else if (dir_num == 4) {
			flags.left = true;
		}
		else if (dir_num == 5) {
			flags.left = true;
			flags.forward = true;
		}
		else if (dir_num == 6) {
			flags.forward = true;
		}
		else if (dir_num == 7) {
			flags.right = true;
			flags.forward = true;
		}
	}

	void movement::set_flags_from_target_direction(const vec2 d) {
		flags.left = flags.right = flags.forward = flags.backward = false;
		if (d.x > 0) flags.right = true;
		if (d.y > 0) flags.backward = true;
		if (d.x < 0) flags.left = true;
		if (d.y < 0) flags.forward = true;
	}

	vec2 movement::get_force_requested_by_input(const vec2& axes) const {
		if (frozen) {
			return vec2::zero;
		}

		return {
			flags.right * axes.x - flags.left * axes.x,
			flags.backward * axes.y - flags.forward * axes.y
		};
	}
	
	void movement::reset_movement_flags() {
		flags = {};
	}
}