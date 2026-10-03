#pragma once
#include <algorithm>
#include <cmath>
#include "augs/math/declare_math.h"

/*
	Logic speed scales the delta passed to the solver while the tickrate stays the same.
	Values outside of this range stop making sense gameplay-wise.
*/

constexpr real32 min_logic_speed_v = 0.5f;
constexpr real32 max_logic_speed_v = 1.0f;

inline real32 sanitize_logic_speed(const real32 speed) {
	if (!std::isfinite(speed)) {
		return max_logic_speed_v;
	}

	const auto clamped = std::clamp(speed, min_logic_speed_v, max_logic_speed_v);
	return std::round(clamped * 100.f) / 100.f;
}
