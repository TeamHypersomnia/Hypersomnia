#pragma once
#include "augs/math/rects.h"

namespace components {
	struct destructible {
		static constexpr bool is_synchronized = true;

		// GEN INTROSPECTOR struct components::destructible
		real32 max_health = -1.0f;
		real32 health = -1.0f;
		xywh texture_rect = xywh(0, 0, 1.0f, 1.0f); // In 0-1 UV space. Defaults to (0, 0, 1.0, 1.0) when never split. Origin (0,0) is top-left.
		// END GEN INTROSPECTOR

		bool is_enabled() const {
			return max_health > 0.0f;
		}
	};
}
