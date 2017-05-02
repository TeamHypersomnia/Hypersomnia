#include "sentience_meter.h"
#include "augs/ensure.h"
#include <algorithm>

bool sentience_meter::is_enabled() const {
	return maximum > 0.f;
}

bool sentience_meter::is_positive() const {
	return value > 0.f;
}

void sentience_meter::set_value(const meter_value_type v) {
	value = static_cast<meter_value_type>(v);
}

void sentience_meter::set_maximum_value(const meter_value_type v) {
	maximum = static_cast<meter_value_type>(v);
}

meter_value_type sentience_meter::get_maximum_value() const {
	return static_cast<meter_value_type>(maximum);
}

meter_value_type sentience_meter::get_value() const {
	return static_cast<meter_value_type>(value);
}

float sentience_meter::get_ratio() const {
	return value / static_cast<float>(maximum);
}

sentience_meter::damage_result sentience_meter::calculate_damage_result(
	const meter_value_type dealt,
	const meter_value_type lower_bound
) const {
	sentience_meter::damage_result result;

	if (dealt > 0) {
		const auto offset_value = std::max(0.f, value - lower_bound);

		if (dealt >= offset_value) {
			result.excessive = dealt - offset_value;
			result.effective = offset_value;
		}
		else {
			result.effective = dealt;
		}
	}
	else {
		ensure(!(lower_bound > 0.f));

		if (value - dealt > maximum) {
			result.effective = -(maximum - value);
		}
		else {
			result.effective = dealt;
		}
	}

	result.ratio_effective_to_maximum = std::abs(result.effective) / static_cast<float>(maximum);

	return result;
}