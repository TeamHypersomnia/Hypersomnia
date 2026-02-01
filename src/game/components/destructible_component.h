#pragma once
#include "augs/math/rects.h"

namespace components {
	struct destructible {
		static constexpr bool is_synchronized = true;

		// GEN INTROSPECTOR struct components::destructible
		real32 max_health = -1.0f;
		real32 health = -1.0f;
		xywh texture_rect = xywh(0, 0, 1.0f, 1.0f); // in 0-1 space. if 0,0,1.0f,1.0f, was never split yet. (0,0) is top-left in UV space.
		// END GEN INTROSPECTOR

		bool is_enabled() const {
			return max_health > 0.0f;
		}
	};
}
