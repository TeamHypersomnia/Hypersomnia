#include "variable_delta_timer.h"

namespace augs {
	variable_delta variable_delta_timer::extract_variable_delta(const fixed_delta_timer& timer) {
		auto extracted = frame_timer.extract<std::chrono::milliseconds>() * timer.get_stepping_speed_multiplier();
		
		auto fixed = timer.get_fixed_delta();

		variable_delta out;
		out.fixed = fixed;
		out.interpolation_ratio = timer.fraction_of_step_until_next_step();
		out.delta_ms = extracted;

		return out;
	}
}