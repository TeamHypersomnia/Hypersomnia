#pragma once
#include <optional>
#include <utility>
#include <algorithm>
#include <cmath>

#include "augs/math/vec2.h"

/*
	Intersects the segment [p1, p2] with a rotated rectangle,
	via a slab test in the rectangle's local space.

	Returns the intersection as a [t_min, t_max] interval
	in the segment's 0-1 parameter space, or std::nullopt on a miss.
*/
inline std::optional<std::pair<real32, real32>> segment_obb_intersection(
	const vec2 p1,
	const vec2 p2,
	const vec2 rect_center,
	const vec2 rect_half_size,
	const real32 rect_rotation_degs
) {
	const auto local_p1 = vec2(p1 - rect_center).rotate(-rect_rotation_degs);
	const auto local_p2 = vec2(p2 - rect_center).rotate(-rect_rotation_degs);
	const auto dir = local_p2 - local_p1;

	auto t_min = 0.f;
	auto t_max = 1.f;

	const auto test_axis = [&](const real32 origin, const real32 axis_dir, const real32 half) {
		if (std::abs(axis_dir) < 1e-6f) {
			/* Parallel to the slab - either always inside it, or never. */
			return std::abs(origin) <= half;
		}

		auto t1 = (-half - origin) / axis_dir;
		auto t2 = (half - origin) / axis_dir;

		if (t1 > t2) {
			std::swap(t1, t2);
		}

		t_min = std::max(t_min, t1);
		t_max = std::min(t_max, t2);

		return true;
	};

	if (!test_axis(local_p1.x, dir.x, rect_half_size.x)) {
		return std::nullopt;
	}

	if (!test_axis(local_p1.y, dir.y, rect_half_size.y)) {
		return std::nullopt;
	}

	if (t_min >= t_max) {
		return std::nullopt;
	}

	return std::pair(t_min, t_max);
}
