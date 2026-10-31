#pragma once
#include <cstddef>
#include <limits>
#include <algorithm>
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

	/*
		Whether the point is within any simple polygon, convex or not (even-odd rule).
	*/

	template <class P>
	bool is_within_polygon(const P& polygon, const vec2 p) {
		const auto n = polygon.size();
		bool inside = false;

		for (std::size_t i = 0, j = n - 1; i < n; j = i++) {
			const auto a = vec2(polygon[i]);
			const auto b = vec2(polygon[j]);

			if ((a.y > p.y) != (b.y > p.y) && p.x < (b.x - a.x) * (p.y - a.y) / (b.y - a.y) + a.x) {
				inside = !inside;
			}
		}

		return inside;
	}

	/*
		The distance from the point to the closest edge of the polygon.
	*/

	template <class P>
	real32 distance_to_polygon_outline(const P& polygon, const vec2 p) {
		const auto n = polygon.size();
		auto closest_sq = std::numeric_limits<real32>::max();

		for (std::size_t i = 0; i < n; ++i) {
			closest_sq = std::min(closest_sq, p.sq_distance_from_segment(vec2(polygon[i]), vec2(polygon[(i + 1) % n])));
		}

		return repro::sqrt(closest_sq);
	}

	/*
		Whether a circle lies within convex polygons taken together: its middle and 8 points around it at its radius.
	*/

	template <class Polygons>
	bool is_circle_within_polygons(const Polygons& polygons, const vec2 center, const real32 radius) {
		auto within = [&](const vec2 p) {
			for (const auto& polygon : polygons) {
				if (is_within_convex_polygon(polygon, p)) {
					return true;
				}
			}

			return false;
		};

		if (!within(center)) {
			return false;
		}

		for (int i = 0; i < 8; ++i) {
			if (!within(center + vec2::from_degrees(45.f * i) * radius)) {
				return false;
			}
		}

		return true;
	}
}
