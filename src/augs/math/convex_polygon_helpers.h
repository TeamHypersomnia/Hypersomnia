#pragma once
#include <cstddef>
#include <array>
#include <limits>

#include "augs/math/vec2.h"

namespace augs {
	inline std::array<vec2, 4> make_rotated_corners(const vec2 size, const real32 rotation_degs) {
		const auto half = size / 2;

		std::array<vec2, 4> corners = {
			vec2(-half.x, -half.y),
			vec2(half.x, -half.y),
			vec2(half.x, half.y),
			vec2(-half.x, half.y)
		};

		for (auto& c : corners) {
			c.rotate(rotation_degs);
		}

		return corners;
	}

	template <class VertsContainer>
	vec2 calc_centroid(const VertsContainer& polygon) {
		auto result = vec2::zero;

		for (const auto& v : polygon) {
			result += vec2(v);
		}

		return result / static_cast<real32>(polygon.size());
	}

	/*
		The outward-facing normal of the polygon's i-th edge,
		disambiguated by the centroid so that the winding does not matter.
	*/
	template <class VertsContainer>
	vec2 outward_normal_of_edge(
		const VertsContainer& polygon,
		const vec2 centroid,
		const std::size_t i
	) {
		const auto a = vec2(polygon[i]);
		const auto b = vec2(polygon[(i + 1) % polygon.size()]);

		auto result = vec2(b.y - a.y, a.x - b.x);

		if (result.dot(centroid - a) > 0.f) {
			result = -result;
		}

		return result.normalize();
	}

	/*
		How far outside the polygon the point lies: the maximum signed distance
		past any of its edges. Negative inside, around zero on the boundary.
		Optionally reports which edge that was.
	*/
	template <class VertsContainer>
	real32 calc_max_edge_violation(
		const VertsContainer& polygon,
		const vec2 centroid,
		const vec2 point,
		std::size_t* const out_worst_edge = nullptr
	) {
		auto result = std::numeric_limits<real32>::lowest();

		for (std::size_t i = 0; i < polygon.size(); ++i) {
			const auto edge_normal = outward_normal_of_edge(polygon, centroid, i);
			const auto dist = edge_normal.dot(point - vec2(polygon[i]));

			if (dist > result) {
				result = dist;

				if (out_worst_edge != nullptr) {
					*out_worst_edge = i;
				}
			}
		}

		return result;
	}
}
