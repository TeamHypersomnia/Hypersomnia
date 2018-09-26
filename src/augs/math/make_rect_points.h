#pragma once
#include "augs/math/vec2.h"

namespace augs {
	template <class T, class S, class D, class P>
	auto make_rect_points(const basic_vec2<S> size, const D degrees, const P& pos) {
		std::array<T, 4> v;

		v[0] = v[1] = v[2] = v[3] = -size / 2;

		v[1].x += size.x;

		v[2].x += size.x;
		v[2].y += size.y;

		v[3].y += size.y;

		const auto radians = DEG_TO_RAD<float> * degrees;

		const auto s = static_cast<float>(sin(radians));
		const auto c = static_cast<float>(cos(radians));

		for (auto& vv : v) {
			const auto new_x = vv.x * c - vv.y * s;
			const auto new_y = vv.x * s + vv.y * c;

			vv.x = new_x;
			vv.y = new_y;
			
			vv += pos;
		}

		return v;
	}
}
