#include "smooth_value_field.h"

namespace augs {
	void smooth_value_field::tick(
		const delta dt, 
		const smoothing_settings<double> settings
	) {
		const double averaging_constant =
			std::pow(settings.average_factor, dt.per_second(settings.averages_per_sec))
		;

		auto calculated_smoothed_value = value * averaging_constant + target_value * (1.0 - averaging_constant);

		if (vec2(calculated_smoothed_value).compare_abs(vec2(target_value), 1.0f)) {
			calculated_smoothed_value = target_value;
		}

		value = calculated_smoothed_value;
	}
}