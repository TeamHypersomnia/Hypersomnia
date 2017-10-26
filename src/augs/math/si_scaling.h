#pragma once

struct si_scaling {
	// GEN INTROSPECTOR struct si_scaling
	float to_meters_multiplier = 0.f;
	float to_pixels_multiplier = 0.f;
	// END GEN INTROSPECTOR

	si_scaling() {
		set_pixels_per_meter(100.f);
	}

	void set_pixels_per_meter(const float pixels) {
		to_meters_multiplier = 1.f / pixels;
		to_pixels_multiplier = pixels;
	}

	template <class T>
	auto get_meters(const T pixels) const {
		return to_meters_multiplier * pixels;
	}

	template <class T>
	auto get_pixels(const T meters) const {
		return to_pixels_multiplier * meters;
	}
};