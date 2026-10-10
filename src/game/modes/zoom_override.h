#pragma once
#include <algorithm>
#include <cmath>
#include "augs/math/declare_math.h"
#include "game/balance_params.h"

/*
	The server's override of the maps' default zoom.
	Values outside of this range stop making sense gameplay-wise.
*/

constexpr real32 min_zoom_override_v = 0.5f;
constexpr real32 max_zoom_override_v = 1.5f;

inline real32 sanitize_zoom_override(const real32 zoom) {
	if (!std::isfinite(zoom)) {
		return BALANCE_ZOOM_OUT;
	}

	const auto clamped = std::clamp(zoom, min_zoom_override_v, max_zoom_override_v);
	return std::round(clamped * 100.f) / 100.f;
}
