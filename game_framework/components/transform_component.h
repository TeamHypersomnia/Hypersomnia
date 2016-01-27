#pragma once
#include "math/rects.h"
#include "math/vec2.h"

namespace components {
	struct transform {
		vec2 pos;
		float rotation = 0.0f;

		transform(vec2 pos = vec2(), float rotation = 0.0f) : pos(pos), rotation(rotation) {}
	};
}