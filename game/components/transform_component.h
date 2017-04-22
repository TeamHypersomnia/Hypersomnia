#pragma once
#include "augs/math/rects.h"
#include "augs/math/vec2.h"

struct b2Sweep;
struct b2Transform;
struct si_scaling;

namespace components {
	struct transform {
		// GEN INTROSPECTOR struct components::transform
		vec2 pos;
		float rotation = 0.0f;
		// END GEN INTROSPECTOR
		
		transform(float x, float y, float rotation = 0.0f);
		transform(vec2 pos = vec2(), float rotation = 0.0f);

		transform operator*(const transform& b) const;

		transform operator+(const transform& b) const;
		transform operator-(const transform& b) const;
		transform& operator+=(const transform& b);
		bool operator==(const transform& b) const;

		void to_box2d_transforms(b2Transform&, b2Sweep&) const;

		transform interpolated(const transform& previous, float ratio, float epsilon = 1.f) const;
		transform interpolated_separate(const transform& previous, float positional_ratio, float rotational_ratio, float epsilon = 1.f) const;

		transform to_si_space(const si_scaling) const;
		transform to_user_space(const si_scaling) const;

		void flip_rotation();
		void reset();
		vec2 interpolation_direction(const transform& previous) const;
	};
}

namespace augs {
	template <class A>
	components::transform interp(components::transform a, components::transform b, A alpha) {
		components::transform res;
		res.pos = augs::interp(a.pos, b.pos, alpha);
		res.rotation = augs::interp(vec2().set_from_degrees(a.rotation), vec2().set_from_degrees(b.rotation), alpha).degrees();
		return res;
	}
}

#include "game/detail/position_scripts.h"
