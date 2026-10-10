#pragma once
#include <cstddef>
#include <utility>
#include <vector>
#include "augs/math/vec2.h"

namespace augs {
	/*
		Ramer-Douglas-Peucker: appends to output the points of the polyline from first_index to last_index, inclusive,
		without those closer than tolerance to the line through their kept neighbours.
		The first and the last point are always kept.
	*/

	inline void simplify_polyline(
		const std::vector<vec2>& points,
		const std::size_t first_index,
		const std::size_t last_index,
		const real32 tolerance,
		std::vector<vec2>& output
	) {
		if (first_index > last_index || last_index >= points.size()) {
			return;
		}

		thread_local std::vector<bool> kept;
		thread_local std::vector<std::pair<std::size_t, std::size_t>> ranges;

		kept.assign(last_index - first_index + 1, false);
		kept.front() = true;
		kept.back() = true;

		ranges.clear();
		ranges.emplace_back(first_index, last_index);

		while (!ranges.empty()) {
			const auto [a, b] = ranges.back();
			ranges.pop_back();

			if (b <= a + 1) {
				continue;
			}

			const auto from = points[a];
			const auto to = points[b];

			auto farthest = a;
			auto farthest_sq_distance = tolerance * tolerance;

			for (auto i = a + 1; i < b; ++i) {
				const auto sq_distance = points[i].sq_distance_from_segment(from, to);

				if (sq_distance > farthest_sq_distance) {
					farthest = i;
					farthest_sq_distance = sq_distance;
				}
			}

			if (farthest != a) {
				kept[farthest - first_index] = true;
				ranges.emplace_back(a, farthest);
				ranges.emplace_back(farthest, b);
			}
		}

		for (auto i = first_index; i <= last_index; ++i) {
			if (kept[i - first_index]) {
				output.push_back(points[i]);
			}
		}
	}
}
