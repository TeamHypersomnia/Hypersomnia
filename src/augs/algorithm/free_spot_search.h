#pragma once
#include <optional>
#include "augs/math/vec2.h"

namespace augs {
	/*
		From a spot along dir, in steps of 1: first until a spot fits, then on past the taken ones, as long as they fit.
		Returns the first free spot that fits, the last one that fits if all of them are taken,
		none if no spot fits within max_distance (or dir is zero).

		Fits - Lambda: (vec2) -> bool
		IsTaken - Lambda: (vec2) -> bool
	*/

	template <class Fits, class IsTaken>
	std::optional<vec2> march_to_free_spot(const vec2 from, const vec2 dir, const real32 max_distance, Fits&& fits, IsTaken&& is_taken) {
		std::optional<vec2> last_fit;

		if (dir.is_zero()) {
			return last_fit;
		}

		for (real32 marched = 0.f; marched <= max_distance; marched += 1.f) {
			const auto candidate = from + dir * marched;

			if (!fits(candidate)) {
				if (last_fit.has_value()) {
					break;
				}

				continue;
			}

			last_fit = candidate;

			if (!is_taken(candidate)) {
				return candidate;
			}
		}

		return last_fit;
	}

	/*
		The nearest spot around another that fits and is free - in rings of growing radius (steps of 1),
		each one checked in 16 directions from start_degrees. None within max_radius.

		Fits - Lambda: (vec2) -> bool
		IsTaken - Lambda: (vec2) -> bool
	*/

	template <class Fits, class IsTaken>
	std::optional<vec2> find_free_spot_around(const vec2 at, const real32 max_radius, const real32 start_degrees, Fits&& fits, IsTaken&& is_taken) {
		constexpr int directions = 16;

		for (real32 radius = 1.f; radius <= max_radius; radius += 1.f) {
			for (int i = 0; i < directions; ++i) {
				const auto candidate = at + vec2::from_degrees(start_degrees + i * (360.f / directions)) * radius;

				if (fits(candidate) && !is_taken(candidate)) {
					return candidate;
				}
			}
		}

		return std::nullopt;
	}
}
