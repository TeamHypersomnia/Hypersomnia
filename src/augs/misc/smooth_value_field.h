#pragma once
#include "augs/math/vec2.h"
#include "augs/misc/timer.h"

namespace augs {
	struct smooth_value_field_settings {
		// GEN INTROSPECTOR struct smooth_value_field_settings
		double averages_per_sec = 0.0;
		double smoothing_average_factor = 0.5;
		// END GEN INTROSPECTOR
	};

	struct smooth_value_field {
		vec2i discrete_value;
		vec2 value;
		vec2 target_value;

		void tick(const double delta_seconds, const smooth_value_field_settings);
	};
}