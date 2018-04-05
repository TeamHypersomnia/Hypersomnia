#pragma once
#include "augs/misc/enum/enum_array.h"

#include "game/container_sizes.h"

namespace invariants {
	struct shape_circle {
		static constexpr bool reinfer_when_tweaking = true;

		// GEN INTROSPECTOR struct invariants::shape_circle
		real32 radius = 0.f;
		// END GEN INTROSPECTOR

		auto get_radius() const {
			return radius;
		}
	};
}
