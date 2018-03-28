#pragma once

struct b2Vec2;

struct si_scaling {
	// GEN INTROSPECTOR struct si_scaling
	int to_pixels_multiplier = 100;
	// END GEN INTROSPECTOR

	void set_pixels_per_meter(const int pixels) {
		to_pixels_multiplier = pixels;
	}

	template <class T>
	auto get_meters(const T pixels) const {
		if constexpr(std::is_same_v<T, b2Vec2>) {
			auto r = pixels;
			r.x /= to_pixels_multiplier;
			r.y /= to_pixels_multiplier;
			return r;
		}
		else {
			return pixels / to_pixels_multiplier;
		}
	}

	template <class T>
	auto get_pixels(const T meters) const {
		if constexpr(std::is_same_v<T, b2Vec2>) {
			auto r = meters;
			r.x *= to_pixels_multiplier;
			r.y *= to_pixels_multiplier;
			return r;
		}
		else {
			return meters * to_pixels_multiplier;
		}
	}
};