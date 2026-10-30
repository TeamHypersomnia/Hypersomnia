#pragma once
#include <cstddef>
#include "augs/math/vec2.h"

namespace augs {
	/*
		Whether the point is within a convex polygon (or on its edge), whichever its winding.
	*/

	template <class P>
	bool is_within_convex_polygon(const P& polygon, const vec2 p) {
		const auto n = polygon.size();

		if (n < 3) {
			return false;
		}

		bool any_positive = false;
		bool any_negative = false;

		for (std::size_t i = 0; i < n; ++i) {
			const auto a = vec2(polygon[i]);
			const auto b = vec2(polygon[(i + 1) % n]);
			const auto side = (b - a).cross(p - a);

			any_positive = any_positive || side > 0.f;
			any_negative = any_negative || side < 0.f;
		}

		return !(any_positive && any_negative);
	}
}
