#pragma once
#include "math/rects.h"
#include "math/vec2.h"

namespace components {
	struct transform {
		vec2 pos;
		float rotation = 0.0f;

		transform(float x, float y, float rotation = 0.0f) : pos(vec2(x, y)), rotation(rotation) {}
		transform(vec2 pos = vec2(), float rotation = 0.0f) : pos(pos), rotation(rotation) {}

		transform operator+(const transform& b) const {
			transform out;
			out.pos = pos + b.pos;
			out.rotation = rotation + b.rotation;
			return out;
		}

		void reset() {
			pos.reset();
			rotation = 0.f;
		}
	};
}