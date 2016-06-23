#include "variable_delta_timer.h"

namespace augs {
	variable_delta variable_delta_timer::extract_variable_delta(const fixed_delta_timer& fixed_delta) {
		auto extracted = frame_timer.extract<std::chrono::milliseconds>() * fixed_delta.get_stepping_speed_multiplier();
		last_frame_timestamp_seconds += extracted / 1000.0;
		
		variable_delta out;
		out.fixed = fixed_delta.get_fixed_delta();
		out.interpolation_ratio = fixed_delta.fraction_of_time_until_the_next_logic_step();
		out.last_frame_timestamp_seconds = last_frame_timestamp_seconds;
		out.variable_delta_ms = extracted;

		return out;
	}
}