#pragma once
#include "fixed_delta_timer.h"
#include <algorithm>
#include "augs/ensure.h"
#undef min
/* credits goes to http://www.unagames.com/blog/daniele/2010/06/fixed-time-step-implementation-box2d */

namespace augs {
	fixed_delta_timer::fixed_delta_timer(unsigned steps_per_second, unsigned max_steps_to_perform)
		: max_steps_to_perform(max_steps_to_perform) {
		basic_delta.steps_per_second = steps_per_second;
		basic_delta.fixed_delta_ms = 1000.0 / steps_per_second;
	}

	unsigned fixed_delta_timer::count_logic_steps_to_perform() {
		accumulator += ticks.extract<std::chrono::milliseconds>() * time_multiplier;

		const unsigned steps = static_cast<unsigned>(std::floor(accumulator / basic_delta.fixed_delta_ms));

		if (steps > 0) 
			accumulator -= steps * basic_delta.fixed_delta_ms;

		ensure(
			"Accumulator must have a value lesser than the fixed time step" &&
			accumulator < basic_delta.fixed_delta_ms + FLT_EPSILON
			);

		return std::min(steps, max_steps_to_perform);
	}

	void fixed_delta_timer::increment_total_steps_passed() {
		++basic_delta.total_steps_passed;
	}

	double fixed_delta_timer::fraction_of_step_until_next_step() const {
		return accumulator / basic_delta.fixed_delta_ms;
	}

	void fixed_delta_timer::set_stepping_speed_multiplier(double tm) {
		time_multiplier = tm;
	}

	double fixed_delta_timer::get_stepping_speed_multiplier() const {
		return time_multiplier;
	}

	fixed_delta fixed_delta_timer::get_fixed_delta() const {
		return basic_delta;
	}
}
