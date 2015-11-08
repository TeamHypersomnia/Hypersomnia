#pragma once

#include "math/vec2.h"
#include "misc/timer.h"

namespace augs {
	namespace misc {
		class smooth_value_field {
		public:
			augs::vec2<int> discrete_value;
			augs::vec2<> value;

			augs::vec2<> target_value;

			double averages_per_sec = 20.0;
			double smoothing_average_factor = 0.5;

			augs::misc::timer delta_timer;

			void tick();
		};
	}
}