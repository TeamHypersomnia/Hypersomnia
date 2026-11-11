#pragma once
#include "augs/math/vec2.h"
#include "augs/math/repro_math.h"

struct movement_orbit_def {
	// GEN INTROSPECTOR struct movement_orbit_def
	real32 radius = 6000.f;
	real32 center_direction_degrees = 90.f;
	real32 degrees_per_sec = 0.1f;
	// END GEN INTROSPECTOR

	bool operator==(const movement_orbit_def&) const = default;

	vec2 calc_center_seen_from(const vec2 start) const {
		return start + vec2::from_degrees(center_direction_degrees) * radius;
	}
};

inline vec2 orbit_position(
	const vec2 center,
	const vec2 start,
	const real32 degrees_per_sec,
	const real32 secs_on_orbit
) {
	const auto from_center = start - center;
	const auto radians = degrees_per_sec * secs_on_orbit * DEG_TO_RAD<real32>;
	const auto cos_angle = repro::cos(radians);
	const auto sin_angle = repro::sin(radians);

	return center + vec2(
		from_center.x * cos_angle - from_center.y * sin_angle,
		from_center.x * sin_angle + from_center.y * cos_angle
	);
}
