#pragma once
#include <algorithm>
#include <cmath>
#include <string>
#include "augs/math/declare_math.h"
#include "augs/misc/timing/stepped_timing.h"
#include "augs/string/typesafe_sprintf.h"

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

/*
	Bounds the tickrate so that a malicious server can't make the client divide by zero
	or hang trying to catch up with thousands of steps per second.
*/

constexpr uint32_t min_tickrate_v = 10;
constexpr uint32_t max_tickrate_v = 240;

inline void sanitize_clock_timing(augs::stepped_clock& clk) {
	const auto tickrate = std::clamp(clk.tickrate, min_tickrate_v, max_tickrate_v);
	clk.set_timing(tickrate, ::sanitize_logic_speed(clk.logic_speed));
	clk.bullet_speed = ::sanitize_logic_speed(clk.bullet_speed);
}

/*
	Applies the requested logic and bullet speeds to the cosmos clock, if they differ and the mode allows changing them now.
	A template so that this header does not need to include the cosmos.
*/

template <class Cosmos, class CanChangeNow>
void apply_requested_logic_speed(
	Cosmos& cosm,
	const real32 requested_speed,
	const real32 requested_bullet_speed,
	CanChangeNow&& can_change_now
) {
	const auto& clk = cosm.get_clock();
	const auto sanitized_speed = ::sanitize_logic_speed(requested_speed);
	const auto sanitized_bullet_speed = ::sanitize_logic_speed(requested_bullet_speed);

	if (clk.logic_speed == sanitized_speed && clk.bullet_speed == sanitized_bullet_speed) {
		return;
	}

	if (can_change_now()) {
		cosm.set_clock_timing(clk.tickrate, sanitized_speed, sanitized_bullet_speed);
	}
}

/*
	At least one and at most two decimal places, e.g. "1.0x", "0.8x", "0.75x".
*/

inline std::string format_logic_speed(const real32 speed) {
	auto result = typesafe_sprintf("%2f", speed);

	if (result.size() > 1 && result.back() == '0') {
		result.pop_back();
	}

	return result + "x";
}

/*
	Round time limits are given for logic speed 1.
	A slowed down game gets proportionally more real time, so that as much can happen in a round.
*/

inline real32 calc_real_round_secs(const uint32_t round_secs, const augs::stepped_clock& clk) {
	return static_cast<real32>(round_secs) / clk.logic_speed;
}
