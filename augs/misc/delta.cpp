#include "delta.h"
#include "stepped_timing.h"

namespace augs {
	float delta::in_seconds() const {
		return delta_ms / 1000.f;
	}

	float delta::in_milliseconds() const {
		return delta_ms;
	}

	fixed_delta::fixed_delta(const unsigned steps_per_second) {
		this->steps_per_second = steps_per_second;
		delta_ms = 1000.0f / steps_per_second;
	}

	unsigned fixed_delta::get_steps_per_second() const {
		return steps_per_second;
	}
	
	float variable_delta::view_interpolation_ratio() const {
		return interpolation_ratio;
	}
	
	fixed_delta variable_delta::get_fixed() const {
		return fixed;
	}
}