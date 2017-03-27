#include "variable_delta_timer.h"

namespace augs {
	delta variable_delta_timer::extract_variable_delta(
		const delta& dt, 
		const fixed_delta_timer& timer
	) {
		return static_cast<float>(frame_timer.extract<std::chrono::milliseconds>() * timer.get_stepping_speed_multiplier());
	}
}