#pragma once
#include "game/components/transform_component.h"

namespace components {
	struct interpolation {
		static constexpr bool is_cache = false;

		// GEN INTROSPECTOR struct components::interpolation
		mutable transformr desired_transform;
		mutable transformr previous_transform;
		mutable transformr interpolated_transform;
		mutable float rotational_slowdown_multiplier = 1.f;
		mutable float positional_slowdown_multiplier = 1.f;
		// END GEN INTROSPECTOR

		template <class T>
		void set_place_of_birth(T&& t) {
			desired_transform = interpolated_transform = std::forward<T>(t);
		}
	};
}

namespace invariants {
	struct interpolation {
		// GEN INTROSPECTOR struct invariants::interpolation
		float base_exponent = 0.9f;
		// END GEN INTROSPECTOR
	};
}
