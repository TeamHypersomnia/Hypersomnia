#pragma once
#include "augs/math/vec2.h"

namespace augs {
	template <class T, class S, class D>
	auto make_rect_points(const basic_vec2<S> size, const D degrees) {
		std::array<T, 4> v;

		v[0] = v[1] = v[2] = v[3] = -size / 2;

		v[1].x += size.x;

		v[2].x += size.x;
		v[2].y += size.y;

		v[3].y += size.y;

		v[0].rotate(degrees);
		v[1].rotate(degrees);
		v[2].rotate(degrees);
		v[3].rotate(degrees);

		return v;
	}
}
