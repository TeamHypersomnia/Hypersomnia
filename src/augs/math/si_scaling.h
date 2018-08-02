#pragma once

struct b2Vec2;
class b2PolygonShape;
class b2CircleShape;

struct si_scaling {
	// GEN INTROSPECTOR struct si_scaling
	int to_pixels_multiplier = 100;
	// END GEN INTROSPECTOR

	void set_pixels_per_meter(const int pixels) {
		to_pixels_multiplier = pixels;
	}

	template <class T>
	auto get_meters(T r) const {
		if constexpr(std::is_same_v<T, b2Vec2>) {
			r.x /= to_pixels_multiplier;
			r.y /= to_pixels_multiplier;
			return r;
		}
		else if constexpr(std::is_same_v<T, b2PolygonShape>) {
			for (int i = 0; i < r.GetVertexCount(); ++i) {
				r.m_vertices[i] /= to_pixels_multiplier;
			}

			return r;
		}
		else {
			return r / to_pixels_multiplier;
		}
	}

	template <class T>
	auto get_pixels(T r) const {
		if constexpr(std::is_same_v<T, b2Vec2>) {
			r.x *= to_pixels_multiplier;
			r.y *= to_pixels_multiplier;
			return r;
		}
		else if constexpr(std::is_same_v<T, b2PolygonShape>) {
			for (int i = 0; i < r.GetVertexCount(); ++i) {
				r.m_vertices[i] *= to_pixels_multiplier;
			}

			return r;
		}
		else {
			return r * to_pixels_multiplier;
		}
	}
};