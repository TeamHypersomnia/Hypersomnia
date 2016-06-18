#include "delta.h"

namespace augs {
	double fixed_delta::in_seconds() const {
		return fixed_delta_ms / 1000.0;
	}

	unsigned fixed_delta::get_steps_per_second() const {
		return steps_per_second;
	}

	double fixed_delta::in_milliseconds() const {
		return fixed_delta_ms;
	}

	double variable_delta::in_seconds() const {
		return variable_delta_ms / 1000.0;
	}

	double variable_delta::in_milliseconds() const {
		return variable_delta_ms;
	}

	double variable_delta::view_interpolation_ratio() const {
		return interpolation_ratio;
	}

	double variable_delta::frame_timestamp_seconds() const {
		return last_frame_timestamp_seconds;
	}
}