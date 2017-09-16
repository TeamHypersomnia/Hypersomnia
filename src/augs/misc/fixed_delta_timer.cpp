#include <algorithm>

#include "augs/ensure.h"
#include "augs/math/vec2.h"
#include "augs/misc/fixed_delta_timer.h"

namespace augs {
	fixed_delta_timer::fixed_delta_timer(const unsigned max_steps_to_perform) 
		: max_steps_to_perform(max_steps_to_perform) 
	{}

	void fixed_delta_timer::advance(const delta frame_delta) {
		accumulator_secs += frame_delta.in_seconds<double>();
	}

	unsigned fixed_delta_timer::extract_num_of_logic_steps(const delta basic_delta) {
		const auto dt_secs = basic_delta.in_seconds<double>();

		const auto steps = std::min(
			max_steps_to_perform,
			static_cast<unsigned>(std::floor(accumulator_secs / dt_secs))
		);

		if (steps > 0) {
			accumulator_secs -= steps * dt_secs;
		}

#if 0
		ensure(
			"Accumulator must have a value lesser than the fixed time step" &&
			accumulator_secs < dt_secs
		);
#endif

		return steps;
	}

	real32 fixed_delta_timer::fraction_of_step_until_next_step(const delta basic_delta) const {
		if (basic_delta.in_seconds<real32>() <= AUGS_EPSILON<real32>) {
			return 0.f;
		}

		return static_cast<float>(accumulator_secs / basic_delta.in_seconds<real32>());
	}
}
