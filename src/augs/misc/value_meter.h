#pragma once
#include "augs/math/declare_math.h"

using meter_value_type = real32;

struct value_meter {
	struct damage_result {
		meter_value_type effective = 0;
		meter_value_type excessive = 0;
		real32 ratio_effective_to_maximum = 0.f;

		meter_value_type total() const;
	};

	// GEN INTROSPECTOR struct value_meter
	meter_value_type value = 100;
	meter_value_type maximum = 100;
	real32 regeneration_unit = 2;
	real32 regeneration_interval_ms = 3000;
	// END GEN INTROSPECTOR

	damage_result calc_damage_result(
		const meter_value_type amount,
		const meter_value_type lower_bound = 0
	) const;

	bool is_enabled() const;
	bool is_positive() const;

	meter_value_type get_maximum_value() const;
	meter_value_type get_value() const;
	void set_value(const meter_value_type);
	void set_maximum_value(const meter_value_type);

	real32 get_ratio() const;
	void make_full();
};