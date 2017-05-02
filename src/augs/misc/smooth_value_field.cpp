#include "smooth_value_field.h"

namespace augs {
	void smooth_value_field::tick(const double delta_seconds) {
		const double averaging_constant =
			pow(smoothing_average_factor, averages_per_sec * delta_seconds)
		;

		auto calculated_smoothed_value = value * averaging_constant + target_value * (1.0 - averaging_constant);

		if (calculated_smoothed_value.compare_abs(target_value, 1.f)) {
			calculated_smoothed_value = target_value;
		}

		value = calculated_smoothed_value;
		discrete_value = value;
	}
}