#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <optional>
#include "augs/math/vec2.h"
#include "augs/math/rects.h"

namespace augs {
	/*
		The bounding box of the part of a rect within an angular sector, together with the apex of the sector.
		The sector starts at the apex and spans half_angle_degrees to both sides of the direction, without end.
		A sector of 180 degrees or more, or without a direction, takes the whole rect.
	*/

	inline ltrb calc_sector_within_rect_aabb(
		const vec2 apex,
		const vec2 direction,
		const real32 half_angle_degrees,
		const ltrb rect
	) {
		auto result = ltrb(apex, vec2::zero);

		auto contain = [&](const vec2 p) {
			result.l = std::min(result.l, p.x);
			result.t = std::min(result.t, p.y);
			result.r = std::max(result.r, p.x);
			result.b = std::max(result.b, p.y);
		};

		if (half_angle_degrees >= 90.0f || direction.is_zero()) {
			contain(rect.lt());
			contain(rect.rb());
			return result;
		}

		const auto dir = vec2(direction).normalize();
		const auto min_dot = repro::cos(half_angle_degrees * DEG_TO_RAD<real32>);

		const std::array<vec2, 4> corners = { rect.lt(), rect.rt(), rect.rb(), rect.lb() };

		for (const auto& c : corners) {
			const auto to_corner = c - apex;
			const auto length = to_corner.length();

			if (length < AUGS_EPSILON<real32> || to_corner.dot(dir) >= min_dot * length) {
				contain(c);
			}
		}

		/*
			Where the sides of the sector cross the sides of the rect.
		*/

		auto ray_segment_hit = [&](const vec2 ray_dir, const vec2 a, const vec2 b) -> std::optional<vec2> {
			const auto edge = b - a;
			const auto denom = ray_dir.cross(edge);

			if (repro::fabs(denom) < AUGS_EPSILON<real32>) {
				return std::nullopt;
			}

			const auto to_a = a - apex;
			const auto t = to_a.cross(edge) / denom;
			const auto s = to_a.cross(ray_dir) / denom;

			if (t >= 0.0f && s >= 0.0f && s <= 1.0f) {
				return apex + ray_dir * t;
			}

			return std::nullopt;
		};

		for (const auto side : { -half_angle_degrees, half_angle_degrees }) {
			const auto side_dir = vec2(dir).rotate(side);

			for (std::size_t i = 0; i < corners.size(); ++i) {
				if (const auto hit = ray_segment_hit(side_dir, corners[i], corners[(i + 1) % corners.size()]); hit.has_value()) {
					contain(*hit);
				}
			}
		}

		return result;
	}
}
