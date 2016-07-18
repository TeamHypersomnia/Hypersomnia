#pragma once

#include "augs/math/vec2.h"
#include "misc/timer.h"

namespace augs {
	class smooth_value_field {
	public:
		vec2i discrete_value;
		vec2 value;

		vec2 target_value;

		double averages_per_sec = 20.0;
		double smoothing_average_factor = 0.5;

		augs::timer delta_timer;

		void tick();
	};
}