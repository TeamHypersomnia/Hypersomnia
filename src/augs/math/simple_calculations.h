#pragma once
#include "augs/math/vec2.h"

namespace augs {
	template<class T>
	auto get_screen_pos_from_offset(const vec2i& screen_size, const T& off, int top_y = 0) {
		auto x = off.x;

		if (x < 0) {
			x = screen_size.x + x;
		}

		auto y = off.y;

		if (y < 0) {
			y = screen_size.y + y;
		}
		else {
			y += top_y;
		}

		return vec2i(x, y);
	}
}

