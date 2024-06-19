#pragma once
#include "augs/math/vec2.h"
#include "augs/misc/timing/delta.h"

namespace augs {
	template <class T>
	struct smoothing_settings {
		// GEN INTROSPECTOR struct augs::smoothing_settings class T
		T averages_per_sec = static_cast<T>(1.0);
		T average_factor = static_cast<T>(0.5);
		// END GEN INTROSPECTOR

		bool operator==(const smoothing_settings<T>& b) const = default;
	};

	struct smooth_value_field {
		vec2d value;
		vec2d target_value;

		void snap_value_to_target() {
			value = target_value;
		}

		void tick(const delta dt, const smoothing_settings<double>);
	};
}