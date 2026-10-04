#pragma once
#include <cstddef>
#include <vector>
#include "augs/math/declare_math.h"

namespace augs {
	/*
		Walks a path of points having a .pos and calls back at every spacing of its length,
		from first_distance up to max_distance, with the point interpolated between the two nearest ones by lerp.

		The callback takes the interpolated point and its distance along the path.
	*/

	template <class P, class Lerp, class F>
	void sample_along_path(
		const std::vector<P>& points,
		const real32 spacing,
		const real32 first_distance,
		const real32 max_distance,
		Lerp&& lerp,
		F&& callback
	) {
		if (points.size() < 2 || spacing <= 0.f) {
			return;
		}

		auto next_distance = first_distance;
		auto segment_start_distance = 0.f;

		for (std::size_t i = 0; i + 1 < points.size(); ++i) {
			const auto& a = points[i];
			const auto& b = points[i + 1];

			const auto segment_length = (b.pos - a.pos).length();
			const auto segment_end_distance = segment_start_distance + segment_length;

			while (next_distance <= segment_end_distance && next_distance <= max_distance) {
				const auto t = segment_length > 0.f ? (next_distance - segment_start_distance) / segment_length : 0.f;

				callback(lerp(a, b, t), next_distance);
				next_distance += spacing;
			}

			if (next_distance > max_distance) {
				return;
			}

			segment_start_distance = segment_end_distance;
		}
	}

	/*
		The length of a path of points having a .pos, up to the point at until_index.
	*/

	template <class P>
	real32 calc_path_length(const std::vector<P>& points, const std::size_t until_index) {
		auto length = 0.f;

		for (std::size_t i = 1; i < points.size() && i <= until_index; ++i) {
			length += (points[i].pos - points[i - 1].pos).length();
		}

		return length;
	}
}
