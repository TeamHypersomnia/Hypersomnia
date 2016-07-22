#pragma once
#include "augs/math/rects.h"
#include "augs/math/vec2.h"

namespace components {
	struct transform {
		vec2 pos;
		float rotation = 0.0f;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(pos),
				CEREAL_NVP(rotation)
			);
		}

		transform(float x, float y, float rotation = 0.0f) : pos(vec2(x, y)), rotation(rotation) {}
		transform(vec2 pos = vec2(), float rotation = 0.0f) : pos(pos), rotation(rotation) {}

		transform operator+(const transform& b) const {
			transform out;
			out.pos = pos + b.pos;
			out.rotation = rotation + b.rotation;
			return out;
		}		
		
		transform operator-(const transform& b) const {
			transform out;
			out.pos = pos - b.pos;
			out.rotation = rotation - b.rotation;
			return out;
		}

		transform& operator+=(const transform& b) {
			(*this) = (*this) + b;
			return *this;
		}

		bool operator==(const transform& b) const {
			return pos == b.pos && rotation == b.rotation;
		}

		void flip_rotation() {
			rotation = -rotation;
		}

		void reset() {
			pos.reset();
			rotation = 0.f;
		}
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
