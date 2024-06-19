#pragma once

enum class interpolation_method {
	// GEN INTROSPECTOR enum class interpolation_method
	NONE,
	LINEAR,
	LINEAR_EXTRAPOLATE,
	EXPONENTIAL,

	COUNT
	// END GEN INTROSPECTOR
};

struct interpolation_settings {
	// GEN INTROSPECTOR struct interpolation_settings
	interpolation_method method = interpolation_method::LINEAR;
	float speed = 525.f;
	// END GEN INTROSPECTOR

	bool operator==(const interpolation_settings& b) const = default;

	bool enabled() const {
		return method != interpolation_method::NONE;
	}
};