#include <algorithm>

#include "augs/math/repro_math.h"
#include "augs/misc/value_meter.h"
#include "augs/ensure.h"

meter_value_type value_meter::damage_result::total() const {
	return effective + excessive;
}

bool value_meter::is_enabled() const {
	return maximum > 0.f;
}

bool value_meter::is_positive() const {
	return value > 0.f;
}

void value_meter::set_value(const meter_value_type v) {
	value = static_cast<meter_value_type>(v);
}

void value_meter::set_maximum_value(const meter_value_type v) {
	maximum = static_cast<meter_value_type>(v);
}

meter_value_type value_meter::get_maximum_value() const {
	return static_cast<meter_value_type>(maximum);
}

meter_value_type value_meter::get_value() const {
	return static_cast<meter_value_type>(value);
}

real32 value_meter::get_ratio() const {
	return value / static_cast<real32>(maximum);
}

value_meter::damage_result value_meter::calc_damage_result(
	const meter_value_type dealt,
	const meter_value_type lower_bound
) const {
	value_meter::damage_result result;

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

	result.ratio_effective_to_maximum = repro::fabs(result.effective) / static_cast<real32>(maximum);

	return result;
}

void value_meter::make_full() {
	value = maximum;
}