#pragma once
#include "smooth_value_field.h"

namespace augs {
	void smooth_value_field::tick() {
		double delta = delta_timer.extract<std::chrono::seconds>();

		double averaging_constant =
			pow(smoothing_average_factor, averages_per_sec * delta);

		value = value * averaging_constant + target_value * (1.0 - averaging_constant);
		discrete_value = value;
	}
}