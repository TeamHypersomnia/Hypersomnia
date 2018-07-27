#pragma once

using meter_value_type = float;

struct value_meter {
	struct damage_result {
		meter_value_type effective = 0;
		meter_value_type excessive = 0;
		float ratio_effective_to_maximum = 0.f;
	};

	// GEN INTROSPECTOR struct value_meter
	meter_value_type value = 100;
	meter_value_type maximum = 100;
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

	float get_ratio() const;
	void make_full();
};