#include "delta.h"
#include "stepped_timing.h"

namespace augs {
	float delta::in_seconds() const {
		return delta_ms / 1000.f;
	}

	float delta::in_milliseconds() const {
		return delta_ms;
	}

	unsigned fixed_delta::get_steps_per_second() const {
		return steps_per_second;
	}
	
	double fixed_delta::total_time_passed_in_seconds() const {
		return total_steps_passed * in_seconds();
	}
	
	unsigned fixed_delta::get_total_steps_passed() const {
		return total_steps_passed;
	}

	stepped_timestamp fixed_delta::get_timestamp() const {
		stepped_timestamp result;
		result.step = total_steps_passed;
		return result;
	}
	
	float variable_delta::view_interpolation_ratio() const {
		return interpolation_ratio;
	}
	
	fixed_delta variable_delta::get_fixed() const {
		return fixed;
	}

	float variable_delta::total_time_passed_in_seconds() const {
		return fixed.total_time_passed_in_seconds() + view_interpolation_ratio() * fixed.in_seconds();
	}
}